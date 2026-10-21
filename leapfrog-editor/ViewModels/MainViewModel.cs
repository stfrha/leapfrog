using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Media;
using System.Net;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Shapes;
using System.Xml;
using System.Xml.Serialization;


namespace LeapfrogEditor
{
   public enum LeftClickState
   {
      none,
      spriteBox,
      spritePolygon,
      staticBox,
      staticCircle,
      staticPolygon,
      staticBoxedSpritePolygon,
      dynamicBox,
      dynamicCircle,
      dynamicPolygon,
      dynamicBoxedSpritePolygon,
      weldJoint,
      revoluteJoint,
      prismaticJoint,
      addPoint,
      rope,
      objectFactory,
      gun,
      shield,
      flameEmitter,
      reentryFlameEmitter
   }

   public enum MouseEventObjectType
   {
      none,                         // object is null
      shape,                        // object is the shape
      dragablePolygonBorder,        // border that a point can be added to,
                                    // object is the point (before or after?) the line
      dragableBorder,               // object is the border of a shape that is not rotated
      rotatableObjectDragableBorder, // object is the border of a shape that can rotate
      dragablePoint,                // object is the point
      compoundObjectBoundaryBox,    // object is the CompoundObject
      jointAnchorA,                 // object is the joint
      jointAnchorB,                 // object is the joint
      prismJointUpperLimit,         // object is the prismatic joint
      prismJointLowerLimit,         // object is the prismatic joint
      objectFactory,                // object is an object factory
      gunSystem,                    // object is an gun system
      flameEmitterSystem,           // object is an flame emitter system
      shieldSystem,                 // object is an shield system
      systemAnchorA,                // object is an reentry flame emitter system
      systemAnchorB                 // object is an reentry flame emitter system
   }

   class MouseEventObjectTypeConverter
   {
      static MouseEventObjectType GetObjectType(object o)
      {
         if (o is LfShapeViewModel)
         {
            return MouseEventObjectType.shape;
         }
         else if (o is ObjectFactoryPropertiesViewModel)
         {
            return MouseEventObjectType.objectFactory;
         }

         return MouseEventObjectType.none;
      }
   }

   public class MainViewModel : MicroMvvm.ViewModelBase
   {

      #region Declarations


      /*
       * In the model-domain, the MyChildObject is the top level entity
       * This contains one state property, i.e. the MyStateProperty.
       * This contains one single child: the MyCP. 
       * The MyCP is connected to the MyCpVm view model
       * This is the data context of the graphic screen.
       * 
       * In the tree view we need the MyCpVm to be the only
       * child in a collection binded to the ItemSource property 
       * of the TreeView. This will be an ObservableCollection of
       * CompoundObjectViewModel, called TreeTop.
       */

      private ObservableCollection<FileCOViewModel> _fileCollectionViewModel = new ObservableCollection<FileCOViewModel>();

      // Object VM that is being edited
      private CompoundObjectViewModel _editedCpVm;

     
      private ObservableCollection<ChildObjectViewModel> _selectedChildObjects = new ObservableCollection<ChildObjectViewModel>();
//      private ObservableCollection<ChildCOViewModel> _selectedChildObjectStateProperties = new ObservableCollection<ChildCOViewModel>();
      private ObservableCollection<LfShapeViewModel> _selectedShapes = new ObservableCollection<LfShapeViewModel>();
      private ObservableCollection<WeldJointViewModel> _selectedJoints = new ObservableCollection<WeldJointViewModel>();
      private ObservableCollection<CoSystemViewModel> _selectedSystems = new ObservableCollection<CoSystemViewModel>();
      private ObservableCollection<LfDragablePointViewModel> _selectedPoints = new ObservableCollection<LfDragablePointViewModel>();
      private SpawnObjectViewModel _editableSpawnObject = null;

      private ZLevels _zLevels = new ZLevels();
      private CollisionEntities _collEnts = new CollisionEntities();

      private LeftClickState _LeftClickState = LeftClickState.none;

      private bool _showJoints = false;
      private bool _showSystems = false;
      private bool _showTriangles = false;
      private bool _showBackgrounds = true;

      private ObservableCollection<string> _textures = new ObservableCollection<string>();

      #endregion

      #region Constructor

      public MainViewModel()
      {
         string fileName = "z_levels.xml";
         string s = GlobalConstants.DataDirectory + fileName;
         string fullPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
         string fullFileName = System.IO.Path.Combine(fullPath, s);

         MyZLevels = ZLevels.ReadFromFile(fullFileName);

         fileName = "collision_entities.xml";
         s = GlobalConstants.DataDirectory + fileName;
         fullPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
         fullFileName = System.IO.Path.Combine(fullPath, s);

         CollEnts = CollisionEntities.ReadFromFile(fullFileName);


         // Build collection of texture names, and keep it up to date while
         // the editor runs.
         UpdateTextureList();
         StartTextureWatcher();
      }

      #endregion

      #region Texture list

      private List<FileSystemWatcher> _textureWatchers = new List<FileSystemWatcher>();
      private string _lastDuplicateWarning = "";
      private System.Windows.Threading.DispatcherTimer _textureRefreshTimer;

      // Syncs Textures with the .png files in the image directories (game
      // images and editor-only images). The collection object is kept, so
      // everything bound to it updates.
      public void UpdateTextureList()
      {
         TextureImages.RebuildIndex();
         List<string> names = new List<string>(TextureImages.TextureNames);
         names.Sort(StringComparer.OrdinalIgnoreCase);

         // The game identifies a texture by file name only, so the same name
         // in two folders is ambiguous. Warn once per new set of duplicates.
         string dups = string.Join("\n", TextureImages.Duplicates);
         if ((dups != "") && (dups != _lastDuplicateWarning))
         {
            _lastDuplicateWarning = dups;
            MessageBox.Show(
               "These texture names exist in more than one folder. The game can't tell them apart; " +
               "rename or remove one of each:\n\n" + dups,
               "Duplicate texture names", MessageBoxButton.OK, MessageBoxImage.Warning);
         }
         else if (dups == "")
         {
            _lastDuplicateWarning = "";
         }

         // Remove textures that are gone
         for (int i = Textures.Count - 1; i >= 0; i--)
         {
            if (!names.Contains(Textures[i], StringComparer.OrdinalIgnoreCase))
            {
               Textures.RemoveAt(i);
            }
         }

         // Add new ones in sorted position
         foreach (string name in names)
         {
            if (!Textures.Contains(name, StringComparer.OrdinalIgnoreCase))
            {
               int pos = 0;
               while ((pos < Textures.Count) && (StringComparer.OrdinalIgnoreCase.Compare(Textures[pos], name) < 0))
               {
                  pos++;
               }
               Textures.Insert(pos, name);
            }
         }
      }

      // Updates the texture list and redraws the scene, so new and
      // re-rendered textures show up.
      public void RefreshTexturesNow()
      {
         UpdateTextureList();

         if (EditedCpVm != null)
         {
            EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);
         }
      }

      private void StartTextureWatcher()
      {
         // MainViewModel is also created by the XAML designer; don't watch there.
         if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(new DependencyObject()))
         {
            return;
         }

         // Blender writes a file in several steps; wait until it has been
         // quiet for a moment before refreshing.
         _textureRefreshTimer = new System.Windows.Threading.DispatcherTimer();
         _textureRefreshTimer.Interval = TimeSpan.FromMilliseconds(500);
         _textureRefreshTimer.Tick += (sender, e) =>
         {
            _textureRefreshTimer.Stop();
            RefreshTexturesNow();
         };

         foreach (string dir in TextureImages.TextureDirectories)
         {
            // editor_images may not exist yet; F5 still finds it later
            if (!Directory.Exists(dir))
            {
               continue;
            }

            FileSystemWatcher w = new FileSystemWatcher(dir, "*.png");
            w.IncludeSubdirectories = true;      // images are organised in subfolders
            w.NotifyFilter = NotifyFilters.FileName | NotifyFilters.LastWrite | NotifyFilters.Size;
            w.Created += OnTextureFileChanged;
            w.Changed += OnTextureFileChanged;
            w.Deleted += OnTextureFileChanged;
            w.Renamed += OnTextureFileChanged;
            w.EnableRaisingEvents = true;
            _textureWatchers.Add(w);
         }
      }

      // Called on a background thread by the FileSystemWatcher.
      private void OnTextureFileChanged(object sender, FileSystemEventArgs e)
      {
         Application app = Application.Current;
         if (app == null)
         {
            return;
         }

         app.Dispatcher.BeginInvoke(new Action(() =>
         {
            // Restart the timer on every event (debounce)
            _textureRefreshTimer.Stop();
            _textureRefreshTimer.Start();
         }));
      }

      #endregion

      #region Properties

      public ObservableCollection<FileCOViewModel> FileCollectionViewModel
      {
         get { return _fileCollectionViewModel; }
         set { _fileCollectionViewModel = value; }
      }

      public CompoundObjectViewModel EditedCpVm
      {
         get { return _editedCpVm; }
         set
         {
            // Keep the last changes to the object being left as an undo step
            // (each edited object has its own undo history)
            if ((_editedCpVm != null) && (_editedCpVm != value))
            {
               UndoCheckpoint();
            }

            _editedCpVm = value;
            OnPropertyChanged("EditedCpVm");
            UpdateEditArea();
            StartUndoHistory();
         }
      }

      // Resize the white edit area to the edited object (see MainWindow.UpdateEditArea)
      public void UpdateEditArea()
      {
         // The scene size may have changed, which limits the parallax window
         ParallaxChanged();

         // Deferred, so it runs after the caller has finished building or
         // loading the object (the bounding box needs its shapes).
         MainWindow mw = System.Windows.Application.Current.MainWindow as MainWindow;
         if (mw != null)
         {
            mw.Dispatcher.BeginInvoke(new Action(mw.UpdateEditArea), System.Windows.Threading.DispatcherPriority.Background);
         }
      }

      public string WindowTitle
      {
         get
         {
            string titleBase = "Leapfrog Editor";

            if (EditedCpVm == null)
            {
               return titleBase +" - No file loaded";
            }

            return EditedCpVm.Name;
         }
      }


      public ObservableCollection<ChildObjectViewModel> SelectedChildObjects
      {
         get { return _selectedChildObjects; }
         set { _selectedChildObjects = value; }
      }

      //public ObservableCollection<ChildCOViewModel> SelectedChildObjectStateProperties
      //{
      //   get { return _selectedChildObjectStateProperties; }
      //   set { _selectedChildObjectStateProperties = value; }
      //}

      public ObservableCollection<LfShapeViewModel> SelectedShapes
      {
         get { return _selectedShapes; }
         set { _selectedShapes = value; }
      }

      public ObservableCollection<WeldJointViewModel> SelectedJoints
      {
         get { return _selectedJoints; }
         set { _selectedJoints = value; }
      }

      public ObservableCollection<CoSystemViewModel> SelectedSystems
      {
         get { return _selectedSystems; }
         set { _selectedSystems = value; }
      }

      public ObservableCollection<LfDragablePointViewModel> SelectedPoints
      {
         get { return _selectedPoints; }
         set { _selectedPoints = value; }
      }

      public SpawnObjectViewModel EditableSpawnObject
      {
         get { return _editableSpawnObject; }
         set
         {
            _editableSpawnObject = value;
            OnPropertyChanged("EditableSpawnObject");
         }
      }

      public LeftClickState LeftClickState
      {
         get { return _LeftClickState; }
         set { _LeftClickState = value; }
      }

      public bool ShowJoints
      {
         get { return _showJoints; }
         set
         {
            _showJoints = value;
            OnPropertyChanged("ShowJoints");

            // Build global shape collection
            EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);
         }
      }

      public bool ShowSystems
      {
         get { return _showSystems; }
         set
         {
            _showSystems = value;
            OnPropertyChanged("ShowSystems");

            // Build global shape collection
            EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);
         }
      }

      public bool ShowBackgrounds
      {
         get { return _showBackgrounds; }
         set
         {
            _showBackgrounds = value;
            OnPropertyChanged("ShowBackgrounds");

            // Build global shape collection
            EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);
         }
      }


      public bool ShowTriangles
      {
         get { return _showTriangles; }
         set
         {
            _showTriangles = value;
            OnPropertyChanged("ShowTriangles");

            // Build global shape collection
            EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);
         }
      }

      public ZLevels MyZLevels
      {
         get { return _zLevels; }
         set
         {
            _zLevels = value;
            OnPropertyChanged("MyZLevels");
         }
      }

      public CollisionEntities CollEnts
      {
         get { return _collEnts; }
         set
         {
            _collEnts = value;
            OnPropertyChanged("CollEnts");
         }
      }

      public ObservableCollection<string> Textures
      {
         get { return _textures; }
         set { _textures = value; }
      }


      #endregion

      #region Commands

      void RefreshTexturesExecute(Object parameter)
      {
         RefreshTexturesNow();
      }

      bool CanRefreshTexturesExecute(Object parameter)
      {
         return true;
      }

      public ICommand RefreshTextures
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => RefreshTexturesExecute(parameter), parameter => CanRefreshTexturesExecute(parameter));
         }
      }

      void NewExecute(Object parameter)
      {
         if (MessageBox.Show("There is no check if there is unsaved data. Do you really want to open a new scene?", "Open Scene Warning", MessageBoxButton.OKCancel, MessageBoxImage.Warning) == MessageBoxResult.OK)
         {
            SaveFileDialog sfd = new SaveFileDialog();

            if (sfd.ShowDialog() == true)
            {
               string fileName = System.IO.Path.GetFileName(sfd.FileName);
               string s = GlobalConstants.DataDirectory + fileName;
               string fullPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
               string fullFileName = System.IO.Path.Combine(fullPath, s);

               ChildObject newChildObject = new ChildObject();
               newChildObject.File = fullFileName;

               CompoundObject newCP = new CompoundObject();
               newChildObject.CompObj = newCP;

               FileCOViewModel newCpVm = new FileCOViewModel(null, null, this, fileName, newCP);

               newCpVm.BuildViewModel();

               FileCollectionViewModel.Add(newCpVm);

               EditedCpVm = newCpVm;
               EditedCpVm.OnPropertyChanged("");
               OnPropertyChanged("");


               // Build global shape collection
               EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);

               Rect bb = EditedCpVm.BoundingBox;
               ((MainWindow)System.Windows.Application.Current.MainWindow).ShowThisRect(bb);


            }
         }
      }

      bool CanNewExecute(Object parameter)
      {
         return true;
      }

      public ICommand New
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => NewExecute(parameter), parameter => CanNewExecute(parameter));
         }
      }

      void ReloadExecute(Object parameter)
      {
         OpenFileDialog ofd = new OpenFileDialog();

         ofd.Filter = "Scene files (*.xml)|*.xml|All files (*.*)|*.*";
         ofd.CheckFileExists = true;

         if (ofd.ShowDialog() == true)
         {
            OpenFileToEdit(System.IO.Path.GetFileName(ofd.FileName), true);
         }
      }

      bool CanReloadExecute(Object parameter)
      {
         return true;
      }

      public ICommand Reload
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => ReloadExecute(parameter), parameter => CanReloadExecute(parameter));
         }
      }

      void ChangeEditableObjectExecute(Object parameter)
      {
         // What object is we pointing at?
         if (parameter is CompoundObjectViewModel)
         {
            DeselectAll();

            EditedCpVm = parameter as CompoundObjectViewModel;
            //EditedCpVm.OnPropertyChanged("");
            //OnPropertyChanged("");

            // Build global shape collection
            EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);

            Rect bb = EditedCpVm.BoundingBox;
            ((MainWindow)System.Windows.Application.Current.MainWindow).ShowThisRect(bb);

         }
      }

      bool CanChangeEditableObjectExecute(Object parameter)
      {
         if (parameter is FileCOViewModel)
         {
            // If the parameter is a file, it is obviously loaded so we want to be able to edit agian
            // if we changed which CO to edit.
            return true;
         }

         if (parameter is ChildCOViewModel)
         {
            ChildCOViewModel covm = parameter as ChildCOViewModel;

            if (!covm.IsFileReferenceChild)
            {
               return true;
            }

         }

         return false;
      }

      public ICommand ChangeEditableObject
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => ChangeEditableObjectExecute(parameter), parameter => CanChangeEditableObjectExecute(parameter));
         }
      }

      void OpenChildFileExecute(Object parameter)
      {
         // What object is we pointing at?
         if (parameter is ChildCOViewModel)
         {
            ChildCOViewModel covm = parameter as ChildCOViewModel;

            OpenFileToEdit(covm.File, true);
         }
      }

      bool CanOpenChildFileExecute(Object parameter)
      {
         if (parameter is ChildCOViewModel)
         {
            ChildCOViewModel covm = parameter as ChildCOViewModel;

            //if (covm == EditedCpVm)
            //{
            //   // We dont want to reopen the file that is already open
            //   // TODO: We need to iterate the file list to know if this 
            //   // file is open
            //   return false;
            //}

            return (covm.IsFileReferenceChild);
         }

         return false;
      }

      public ICommand OpenChildFile
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => OpenChildFileExecute(parameter), parameter => CanOpenChildFileExecute(parameter));
         }
      }

      void SaveThisObjectExecute(Object parameter)
      {
         if (parameter is FileCOViewModel)
         {
            FileCOViewModel fcovm = parameter as FileCOViewModel;

            UndoCheckpoint();

            // Generate Triangles before saving
            fcovm.GenerateTriangles();

            fcovm.ModelObject.WriteToFile(fcovm.FullPathFileName);

            UndoRebaseline(fcovm);

            // Now, since we potentially have changed the contents of this file,
            // lets look if any object has this object as a child, in which case we 
            // update that child (by reloading it).
            UpdateFileReferences(fcovm);
         }
      }

      bool CanSaveThisObjectExecute(Object parameter)
      {
         return (parameter is FileCOViewModel);
      }

      public ICommand SaveThisObject
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => SaveThisObjectExecute(parameter), parameter => CanSaveThisObjectExecute(parameter));
         }
      }

      void CloseThisFileExecute(Object parameter)
      {
         if (parameter is FileCOViewModel)
         {
            FileCOViewModel fcovm = parameter as FileCOViewModel;

            int i = FileCollectionViewModel.IndexOf(fcovm);

            if (i >= 0)
            {
               if (MessageBox.Show("There is no check if there is unsaved data. Do you really want to close this file?", "Close File Warning", MessageBoxButton.OKCancel, MessageBoxImage.Warning) == MessageBoxResult.OK)
               {
                  FileCollectionViewModel.RemoveAt(i);
               }
            }
         }
      }

      bool CanCloseThisFileExecute(Object parameter)
      {
         return (parameter is FileCOViewModel);
      }

      public ICommand CloseThisFile
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => CloseThisFileExecute(parameter), parameter => CanCloseThisFileExecute(parameter));
         }
      }

      void SaveExecute(Object parameter)
      {
         // Generate Triangles before saving
         if (EditedCpVm is FileCOViewModel)
         {
            FileCOViewModel fvm = EditedCpVm as FileCOViewModel;

            UndoCheckpoint();

            fvm.GenerateTriangles();

            fvm.ModelObject.WriteToFile(fvm.FullPathFileName);

            UndoRebaseline(fvm);

            // Now, since we potentially have changed the contents of this file,
            // lets look if any object has this object as a child, in which case we 
            // update that child (by reloading it).
            UpdateFileReferences(fvm);
         }

      }

      bool CanSaveExecute(Object parameter)
      {
         return (EditedCpVm is FileCOViewModel);
      }

      public ICommand Save
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => SaveExecute(parameter), parameter => CanSaveExecute(parameter));
         }
      }

      void SaveAsExecute(Object parameter)
      {
         // Generate Triangles before saving
         if (EditedCpVm is FileCOViewModel)
         {
            FileCOViewModel fvm = EditedCpVm as FileCOViewModel;

            SaveFileDialog sfd = new SaveFileDialog();

            if (sfd.ShowDialog() == true)
            {
               UndoCheckpoint();

               // Generate Triangles before saving
               fvm.GenerateTriangles();
               fvm.FileName = sfd.SafeFileName;
               fvm.ModelObject.WriteToFile(fvm.FullPathFileName);

               UndoRebaseline(fvm);
            }
         }
      }

      bool CanSaveAsExecute(Object parameter)
      {
         return (EditedCpVm is FileCOViewModel);
      }

      public ICommand SaveAs
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => SaveAsExecute(parameter), parameter => CanSaveAsExecute(parameter));
         }
      }

      void ImportCompoundObjectExecute(Object parameter)
      {
         OpenFileDialog ofd = new OpenFileDialog();

         ofd.Filter = "Compound Object files (*.xml)|*.xml|All files (*.*)|*.*";
         ofd.CheckFileExists = true;

         if (ofd.ShowDialog() == true)
         {
            string fileName = System.IO.Path.GetFileName(ofd.FileName);
            string s = GlobalConstants.DataDirectory + fileName;
            string fullPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
            string fullFileName = System.IO.Path.Combine(fullPath, s);

            CompoundObject newCp = CompoundObject.ReadFromFile(fullFileName);

            FileCOViewModel newCpVm = new FileCOViewModel(EditedCpVm.ImportedObjects, EditedCpVm, this, fileName, newCp);

            newCpVm.BuildViewModel();

            EditedCpVm.ImportedObjects.Children.Add(newCpVm);

            EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);

            EditedCpVm.OnPropertyChanged("");
            OnPropertyChanged("");

            Rect bib = EditedCpVm.BoundingBox;
            ((MainWindow)System.Windows.Application.Current.MainWindow).ShowThisRect(bib);
         }

      }

      bool CanImportCompoundObjectExecute(Object parameter)
      {
         return (EditedCpVm != null);
      }

      public ICommand ImportCompoundObject
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => ImportCompoundObjectExecute(parameter), parameter => CanImportCompoundObjectExecute(parameter));
         }
      }


      void PublishExecute(Object parameter)
      {

         // Generate Triangles before saving
         if (EditedCpVm is FileCOViewModel)
         {
            FileCOViewModel fvm = EditedCpVm as FileCOViewModel;

            // Generate Triangles before saving
            fvm.GenerateTriangles();
            fvm.ModelObject.WriteToFile("updated_scene.xml");

            NetworkCredential login = ReadPublishLogin();
            if (login == null)
            {
               MessageBox.Show(
                  "No FTP login for publishing. Create the file\n\n" + PublishLoginFile +
                  "\n\nwith the user name on the first line and the password on the second, " +
                  "or set the environment variables LEAPFROG_FTP_USER and LEAPFROG_FTP_PASSWORD.",
                  "Publish Scene", MessageBoxButton.OK, MessageBoxImage.Information);
               return;
            }

            using (var client = new WebClient())
            {
               client.Credentials = login;
               byte[] responseArray = client.UploadFile("ftp://ftp.fredrikhoffman.se/leapfrog/updated_scene.xml", WebRequestMethods.Ftp.UploadFile, "updated_scene.xml");

               Console.WriteLine("\nResponse Received. The contents of the file uploaded are:\n{0}", System.Text.Encoding.ASCII.GetString(responseArray));
            }

         }
      }

      // The FTP login for Publish is kept out of the repository: in
      // %APPDATA%\LeapfrogEditor\publish_login.txt (user name on the first
      // line, password on the second), or in the environment variables
      // LEAPFROG_FTP_USER and LEAPFROG_FTP_PASSWORD.
      private static string PublishLoginFile
      {
         get
         {
            return System.IO.Path.Combine(
               Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
               "LeapfrogEditor", "publish_login.txt");
         }
      }

      private static NetworkCredential ReadPublishLogin()
      {
         string user = Environment.GetEnvironmentVariable("LEAPFROG_FTP_USER");
         string password = Environment.GetEnvironmentVariable("LEAPFROG_FTP_PASSWORD");

         if (string.IsNullOrEmpty(user) || string.IsNullOrEmpty(password))
         {
            try
            {
               if (!File.Exists(PublishLoginFile))
               {
                  return null;
               }

               string[] lines = File.ReadAllLines(PublishLoginFile);
               if (lines.Length < 2)
               {
                  return null;
               }

               user = lines[0].Trim();
               password = lines[1].Trim();
            }
            catch (Exception)
            {
               return null;
            }
         }

         if (string.IsNullOrEmpty(user) || string.IsNullOrEmpty(password))
         {
            return null;
         }

         return new NetworkCredential(user, password);
      }

      bool CanPublishExecute(Object parameter)
      {
         return (EditedCpVm is FileCOViewModel);
      }

      public ICommand Publish
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => PublishExecute(parameter), parameter => CanPublishExecute(parameter));
         }
      }

      //void DisplayStateExecute(Object parameter)
      //{
      //   if (parameter is StateViewModel)
      //   {
      //      StateViewModel svm = parameter as StateViewModel;

      //      svm.SetOnDisplay();

      //      EditedCpVm.OnPropertyChanged("");
      //      EditedCpVm.BuildGlobalShapeCollection(svm.StateName, ShowJoints, ShowSystems, ShowBackgrounds);
      //   }
      //}

      //bool CanDisplayStateExecute(Object parameter)
      //{
      //   if (parameter is StateViewModel)
      //   {
      //      StateViewModel svm = parameter as StateViewModel;

      //      if (!svm.IsOnDisplay)
      //      {
      //         return true;
      //      }
      //   }

      //   return false;
      //}

      //public ICommand DisplayState
      //{
      //   get
      //   {
      //      return new MicroMvvm.RelayCommand<Object>(parameter => DisplayStateExecute(parameter), parameter => CanDisplayStateExecute(parameter));
      //   }
      //}

      void SetShapeWidthExecute(Object parameter)
      {
         if (parameter is IWidthHeightInterface)
         {
            IWidthHeightInterface iwh = (IWidthHeightInterface)parameter;

            iwh.SetWidthToTextureAspectRatio();
         }
      }

      bool CanSetShapeWidthExecute(Object parameter)
      {
         return true;
      }

      public ICommand SetShapeWidth
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => SetShapeWidthExecute(parameter), parameter => CanSetShapeWidthExecute(parameter));
         }
      }

      void SetShapeHeightExecute(Object parameter)
      {
         if (parameter is IWidthHeightInterface)
         {
            IWidthHeightInterface iwh = (IWidthHeightInterface)parameter;

            iwh.SetHeightToTextureAspectRatio();
         }
      }

      bool CanSetShapeHeightExecute(Object parameter)
      {
         return true;
      }

      public ICommand SetShapeHeight
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => SetShapeHeightExecute(parameter), parameter => CanSetShapeHeightExecute(parameter));
         }
      }

      void SetShapeSizeFromTextureExecute(Object parameter)
      {
         if (parameter is IWidthHeightInterface)
         {
            IWidthHeightInterface iwh = (IWidthHeightInterface)parameter;

            iwh.SetSizeFromTexture();
         }
      }

      bool CanSetShapeSizeFromTextureExecute(Object parameter)
      {
         return true;
      }

      public ICommand SetShapeSizeFromTexture
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => SetShapeSizeFromTextureExecute(parameter), parameter => CanSetShapeSizeFromTextureExecute(parameter));
         }
      }

      void CreateTrianglesExecute(Object parameter)
      {
         EditedCpVm.GenerateTriangles();
      }

      bool CanCreateTrianglesExecute(Object parameter)
      {
         return true;
      }

      public ICommand CreateTriangles
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => CreateTrianglesExecute(parameter), parameter => CanCreateTrianglesExecute(parameter));
         }
      }

      void NewShapeExecute(Object parameter)
      {
         DeselectAll();

         string param = parameter as string;

         if (!Enum.TryParse<LeftClickState>(param,  out _LeftClickState))
         {
            _LeftClickState = LeftClickState.none;
         }
      }

      bool CanNewShapeExecute(Object parameter)
      {
         return ((EditedCpVm != null) && (parameter is string));
      }

      public ICommand NewShape
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => NewShapeExecute(parameter), parameter => CanNewShapeExecute(parameter));
         }
      }

      void NewJointExecute(Object parameter)
      {
         string param = parameter as string;

         if (!Enum.TryParse<LeftClickState>(param, out _LeftClickState))
         {
            _LeftClickState = LeftClickState.none;
         }
         else
         {
            ShowJoints = true;
         }
      }

      bool CanNewJointExecute(Object parameter)
      {
         if (parameter is string)
         {
            string p = (string)parameter;

            if (p == "rope")
            {
               return ((EditedCpVm != null) && (SelectedShapes.Count > 0) && (SelectedShapes.Count < 3));
            }
            else
            {
               return ((EditedCpVm != null) && (SelectedShapes.Count == 2));
            }
         }

         return false;
      }

      public ICommand NewJoint
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => NewJointExecute(parameter), parameter => CanNewJointExecute(parameter));
         }
      }

      void NewSystemExecute(Object parameter)
      {
         string param = parameter as string;

         if (!Enum.TryParse<LeftClickState>(param, out _LeftClickState))
         {
            _LeftClickState = LeftClickState.none;
         }
         else
         {
            ShowSystems = true;
         }
      }

      bool CanNewSystemExecute(Object parameter)
      {
         if (parameter is string)
         {
            string p = (string)parameter;

            if (p == "objectFactory")
            {
               return ((EditedCpVm != null) && (SelectedShapes.Count >= 0) && (SelectedShapes.Count < 1));
            }
            else
            {
               return ((EditedCpVm != null) && (SelectedShapes.Count == 1));
            }
         }

         return false;
      }

      public ICommand NewSystem
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => NewSystemExecute(parameter), parameter => CanNewSystemExecute(parameter));
         }
      }

      void NewSpawnObjectExecute(Object parameter)
      {
         if (parameter is ISpawnObjectParentVmInterface)
         {
            ISpawnObjectParentVmInterface bovm = parameter as ISpawnObjectParentVmInterface;

            // First add all model elements
            SpawnObject newSo = new SpawnObject();
            ChildObject cho = new ChildObject();
            newSo.MyChildObject = cho;

            CompoundObject co = new CompoundObject();
            cho.CompObj = co;

            bovm.SpawnParentModelObject.SpawnObjects.Add(newSo);

            // Then add and link ViewModel elements
            SpawnObjectViewModel sovm = new SpawnObjectViewModel(bovm.ParentVm, EditedCpVm as CompoundObjectViewModel, this, bovm, newSo);
            bovm.SpawnObjects.Add(sovm);
         }
      }

      bool CanNewSpawnObjectExecute(Object parameter)
      {
         if (parameter is ISpawnObjectParentVmInterface)
         {
            return true;
         }
         return false;
      }

      public ICommand NewSpawnObject
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => NewSpawnObjectExecute(parameter), parameter => CanNewSpawnObjectExecute(parameter));
         }
      }

      void DeleteExecute(Object parameter)
      {
         if (_selectedPoints.Count > 0)
         {
            foreach (LfDragablePointViewModel dp in _selectedPoints)
            {
               LfPolygonViewModel polyVm = dp.Parent;

               // Is this the last point to be removed? If so, remove the shape
               // first so there is no problem with updating something with zero
               // points.
               if (polyVm.PointVms.Count == 1)
               {
                  // Polygon has no more points, delete the polygon Shape

                  polyVm.ParentVm.ModelObject.RemoveShape(polyVm.ModelObject);
                  polyVm.ParentVm.ShapeCollection.Shapes.Remove(polyVm);
               }

               // Before we remove the point
               polyVm.RemovePoint(dp);

            }
            _selectedPoints.Clear();
         }
         else if (SelectedJoints.Count > 0)
         {
            foreach (WeldJointViewModel jvm in SelectedJoints)
            {
               CompoundObjectViewModel covm = jvm.ParentVm;

               covm.ModelObject.RemoveJoint(jvm.ModelObject);
               covm.JointCollection.Joints.Remove(jvm);

            }
            SelectedJoints.Clear();

            EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);
         }
         else if (SelectedShapes.Count > 0)
         {
            foreach (LfShapeViewModel svm in SelectedShapes)
            {
               CompoundObjectViewModel covm = svm.ParentVm;

               //covm.ModelObject.RemoveShape(svm.ModelObject);
               covm.RemoveShape(svm);

               EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);
            }
            SelectedShapes.Clear();
         }
      }

      bool CanDeleteExecute(Object parameter)
      {
         return ((EditedCpVm != null) && 
            ((SelectedShapes.Count > 0) || (_selectedPoints.Count > 0)  || (SelectedJoints.Count > 0) ));
      }

      public ICommand Delete
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => DeleteExecute(parameter), parameter => CanDeleteExecute(parameter));
         }
      }

      void DeleteThisObjectExecute(Object parameter)
      {
         if (parameter is WeldJointViewModel)
         {
            WeldJointViewModel jvm = parameter as WeldJointViewModel;
            CompoundObjectViewModel covm = jvm.ParentVm;

            covm.ModelObject.RemoveJoint(jvm.ModelObject);
            covm.JointCollection.Joints.Remove(jvm);
         }

         if (parameter is LfShapeViewModel)
         {
            LfShapeViewModel svm = parameter as LfShapeViewModel;
            CompoundObjectViewModel covm = svm.ParentVm;

            //covm.ModelObject.RemoveShape(svm.ModelObject);
            covm.RemoveShape(svm);
         }

         if (parameter is ChildObjectViewModel)
         {
            ChildObjectViewModel chvm = parameter as ChildObjectViewModel;
            CompoundObjectViewModel covm = chvm.ParentVm as CompoundObjectViewModel;

            covm.ModelObject.ChildObjects.Remove(chvm.ModelObject);
            covm.ChildObjectCollection.Children.Remove(chvm);
         }

         if (parameter is SpawnObjectViewModel)
         {
            SpawnObjectViewModel sovm = parameter as SpawnObjectViewModel;
            ISpawnObjectParentVmInterface spvm = sovm.SpawnParent;

            spvm.SpawnParentModelObject.SpawnObjects.Remove(sovm.LocalModelObject);
            spvm.SpawnObjects.Remove(sovm);

         }

         if (parameter is SystemViewModelBase)
         {
            SystemViewModelBase spvm = parameter as SystemViewModelBase;
            CoSystemViewModel sysVm = spvm.SystemViewModel;
            CompoundObjectViewModel covm = sysVm.ParentVm as CompoundObjectViewModel;

            covm.ModelObject.Systems.Remove(sysVm.LocalModelObject);
            covm.SystemCollection.Systems.Remove(sysVm);
         }

         if (parameter is CoSystemViewModel)
         {
            CoSystemViewModel sysVm = parameter as CoSystemViewModel;
            CompoundObjectViewModel covm = sysVm.ParentVm as CompoundObjectViewModel;

            covm.ModelObject.Systems.Remove(sysVm.LocalModelObject);
            covm.SystemCollection.Systems.Remove(sysVm);
         }

         EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);
      }

      bool CanDeleteThisObjectExecute(Object parameter)
      {
         if (parameter == null)
         {
            return false;
         }

         if ((EditedCpVm != null) && ((parameter is LfShapeViewModel) || 
            (parameter is WeldJointViewModel) || (parameter is ChildObjectViewModel) ||
            (parameter is ChildObjectViewModel) || (parameter is SpawnObjectViewModel) ||
            (parameter is SystemViewModelBase) || (parameter is CoSystemViewModel)))
         {
            return true;
         }
        
         return false;
      }

      public ICommand DeleteThisObject
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => DeleteThisObjectExecute(parameter), parameter => CanDeleteThisObjectExecute(parameter));
         }
      }

      void AddChildObjectExecute(Object parameter)
      {
         ChildObject ch = new ChildObject();
         ch.CompObj = new CompoundObject();
         ChildObjectViewModel chvm = new ChildObjectViewModel(EditedCpVm.ChildObjectCollection, EditedCpVm as CompoundObjectViewModel, this, ch, true);

         CompoundObjectViewModel covm = EditedCpVm as CompoundObjectViewModel;
         covm.ModelObject.ChildObjects.Add(ch);
         EditedCpVm.ChildObjectCollection.Children.Add(chvm);
         EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);
      }

      bool CanAddChildObjectExecute(Object parameter)
      {
         if ((EditedCpVm != null) && (EditedCpVm is CompoundObjectViewModel))
         {
            return true;
         }

         return false;
      }

      // Parameter is ignored in this case
      public ICommand AddChildObject
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => AddChildObjectExecute(parameter), parameter => CanAddChildObjectExecute(parameter));
         }
      }

      void AddChildObjectFileExecute(Object parameter)
      {
         OpenFileDialog ofd = new OpenFileDialog();

         ofd.Filter = "Child object files (*.xml)|*.xml|All files (*.*)|*.*";
         ofd.CheckFileExists = true;

         if (ofd.ShowDialog() == true)
         {
            string s = GlobalConstants.DataDirectory + System.IO.Path.GetFileName(ofd.FileName);
            string fullPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
            string fullFileName = System.IO.Path.Combine(fullPath, s);

            ChildObject ch = new ChildObject();
            ch.File = System.IO.Path.GetFileName(ofd.FileName);
            ch.CompObj = CompoundObject.ReadFromFile(fullFileName);
            ChildObjectViewModel chvm = new ChildObjectViewModel(EditedCpVm.ChildObjectCollection, EditedCpVm as CompoundObjectViewModel, this, ch, true);
            chvm.CompObj.BuildViewModel(true);

            CompoundObjectViewModel covm = EditedCpVm as CompoundObjectViewModel;
            covm.ModelObject.ChildObjects.Add(ch);
            EditedCpVm.ChildObjectCollection.Children.Add(chvm);
            EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);
         }
      }

      bool CanAddChildObjectFileExecute(Object parameter)
      {
         if ((EditedCpVm != null) && (EditedCpVm is CompoundObjectViewModel))
         {
            return true;
         }

         return false;
      }

      public ICommand AddChildObjectFile
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => AddChildObjectFileExecute(parameter), parameter => CanAddChildObjectFileExecute(parameter));
         }
      }

      #region Parallax view

      // The parallax view shows the scene as the game does through a window
      // the size of the game's screen: parallax layers (sprites directly in
      // the scene with parallaxAmount > 0) are moved and scaled for where
      // the window is, as if the Leapfrog were in its centre.

      // Visible stage width at the game's default zoom: the zoom is set to
      // view port width / 3840 and the stage is scaled x10 (SceneActor), so
      // 384 units across on any screen. 220 is the width when landed on a
      // pad (LandingPad, c_landedVisibleWidth).
      public const double ParallaxDefaultWidth = 384.0;
      public const double ParallaxLandedWidth = 220.0;

      private bool _parallaxView = false;
      private double _parallaxWidth = ParallaxDefaultWidth;
      private double _parallaxAspect = 2.0;          // width / height; phones are about 2:1
      private double _parallaxCenterX = double.NaN;
      private double _parallaxCenterY = double.NaN;

      public bool ParallaxView
      {
         get { return _parallaxView; }
         set
         {
            if (_parallaxView == value)
            {
               return;
            }

            _parallaxView = value;

            if (_parallaxView && (double.IsNaN(_parallaxCenterX) || double.IsNaN(_parallaxCenterY)))
            {
               Size stage = ParallaxStageSize;
               _parallaxCenterX = stage.Width / 2;
               _parallaxCenterY = stage.Height / 2;
            }

            OnPropertyChanged("ParallaxView");
            ParallaxChanged();
         }
      }

      // Size of the edited scene (its sceneProperties), or a default size
      // if the edited object isn't a scene
      public Size ParallaxStageSize
      {
         get
         {
            ScenePropertiesViewModel sp = null;
            if ((EditedCpVm != null) && (EditedCpVm.Behaviour != null))
            {
               sp = EditedCpVm.Behaviour.BehaviourProperties as ScenePropertiesViewModel;
            }

            if ((sp != null) && (sp.Width > 0) && (sp.Height > 0))
            {
               return new Size(sp.Width, sp.Height);
            }

            return new Size(1000, 500);
         }
      }

      // The window in scene coordinates. Like the game, it can't be larger
      // than the stage (the zoom stops when the whole stage fits) and it
      // stays inside the stage (the pan limit).
      public Rect ParallaxWindowRect
      {
         get
         {
            Size stage = ParallaxStageSize;
            double w = _parallaxWidth;
            double h = _parallaxWidth / _parallaxAspect;
            double f = Math.Max(1.0, Math.Max(w / stage.Width, h / stage.Height));
            w /= f;
            h /= f;

            double cx = double.IsNaN(_parallaxCenterX) ? stage.Width / 2 : _parallaxCenterX;
            double cy = double.IsNaN(_parallaxCenterY) ? stage.Height / 2 : _parallaxCenterY;
            cx = Math.Min(Math.Max(cx, w / 2), stage.Width - w / 2);
            cy = Math.Min(Math.Max(cy, h / 2), stage.Height - h / 2);

            return new Rect(cx - w / 2, cy - h / 2, w, h);
         }
      }

      // The window's centre, where the Leapfrog would be
      public Point ParallaxWindowCenter
      {
         get
         {
            Rect r = ParallaxWindowRect;
            return new Point(r.X + r.Width / 2, r.Y + r.Height / 2);
         }
      }

      // Everything the parallax transforms depend on, as one value. A new
      // object every time something changes, so bindings to it update.
      public ParallaxViewState ParallaxState
      {
         get
         {
            Size stage = ParallaxStageSize;
            return new ParallaxViewState(_parallaxView, ParallaxWindowRect, stage.Width, stage.Height);
         }
      }

      public void MoveParallaxWindow(Vector delta)
      {
         // Start from the clamped position, so dragging back from beyond
         // an edge moves the window straight away
         Point c = ParallaxWindowCenter;
         SetParallaxWindowCenter(new Point(c.X + delta.X, c.Y + delta.Y));
      }

      public void SetParallaxWindowCenter(Point center)
      {
         _parallaxCenterX = center.X;
         _parallaxCenterY = center.Y;
         ParallaxChanged();
      }

      // Everything outside the window, for darkening it
      public System.Windows.Media.Geometry ParallaxMaskGeometry
      {
         get
         {
            Rect window = ParallaxWindowRect;
            Rect all = new Rect(window.X - 100000, window.Y - 100000, 200000 + window.Width, 200000 + window.Height);
            System.Windows.Media.Geometry g = new System.Windows.Media.CombinedGeometry(
               System.Windows.Media.GeometryCombineMode.Exclude,
               new System.Windows.Media.RectangleGeometry(all),
               new System.Windows.Media.RectangleGeometry(window));
            g.Freeze();
            return g;
         }
      }

      public ICommand ToggleParallaxView
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => { ParallaxView = !ParallaxView; }, parameter => true);
         }
      }

      private void ParallaxChanged()
      {
         OnPropertyChanged("ParallaxMaskGeometry");
         OnPropertyChanged("ParallaxWindowRect");
         OnPropertyChanged("ParallaxWindowCenter");
         OnPropertyChanged("ParallaxState");
         OnPropertyChanged("ParallaxWindowDescription");
      }

      public string ParallaxWindowDescription
      {
         get
         {
            Rect r = ParallaxWindowRect;
            return string.Format(System.Globalization.CultureInfo.InvariantCulture,
               "{0:0} x {1:0} at ({2:0}, {3:0})", r.Width, r.Height, r.X + r.Width / 2, r.Y + r.Height / 2);
         }
      }

      // Parameter: "default" (the game's normal zoom), "landed" (zoomed in
      // on a pad), "fit" (zoomed out as far as the game allows), "wider" /
      // "narrower" (one zoom step), "aspect2" (2:1, phones), "aspect15"
      // (3:2, the 960 x 640 window).
      void ParallaxWindowSizeExecute(Object parameter)
      {
         string p = parameter as string;
         Size stage = ParallaxStageSize;

         switch (p)
         {
            case "default": _parallaxWidth = ParallaxDefaultWidth; break;
            case "landed": _parallaxWidth = ParallaxLandedWidth; break;
            case "fit": _parallaxWidth = Math.Max(stage.Width, stage.Height * _parallaxAspect); break;
            case "wider": _parallaxWidth = Math.Min(ParallaxWindowRect.Width * 1.25, Math.Max(stage.Width, stage.Height * _parallaxAspect)); break;
            case "narrower": _parallaxWidth = Math.Max(ParallaxWindowRect.Width / 1.25, 20.0); break;
            case "aspect2": _parallaxAspect = 2.0; break;
            case "aspect15": _parallaxAspect = 1.5; break;
         }

         ParallaxChanged();
      }

      public ICommand ParallaxWindowSize
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => ParallaxWindowSizeExecute(parameter), parameter => true);
         }
      }

      #endregion

      #region Undo and redo

      // One history per edited object (see UndoHistory). Weak, so histories
      // of objects that are closed or rebuilt go away with them.
      private System.Runtime.CompilerServices.ConditionalWeakTable<CompoundObjectViewModel, UndoHistory> _undoHistories =
         new System.Runtime.CompilerServices.ConditionalWeakTable<CompoundObjectViewModel, UndoHistory>();

      private bool _restoringUndo = false;

      private UndoHistory HistoryOf(CompoundObjectViewModel covm, bool create)
      {
         if ((covm == null) || (covm.ModelObject == null))
         {
            return null;
         }

         UndoHistory h;
         if (!_undoHistories.TryGetValue(covm, out h) && create)
         {
            h = new UndoHistory(covm.ModelObject);
            _undoHistories.Add(covm, h);
         }

         return h;
      }

      // The edited object as it is when editing begins is the first state.
      // Deferred until the caller has built the view models (which may
      // tidy the model).
      private void StartUndoHistory()
      {
         System.Windows.Application app = System.Windows.Application.Current;
         if (app == null)
         {
            return;
         }

         CompoundObjectViewModel covm = EditedCpVm;
         app.Dispatcher.BeginInvoke(new Action(() =>
         {
            if (covm == EditedCpVm)
            {
               try
               {
                  // A history already there (the object was edited before)
                  // takes changes made meanwhile as a step
                  UndoHistory h = HistoryOf(covm, false);
                  if (h != null)
                  {
                     h.Checkpoint(covm.ModelObject);
                  }
                  else
                  {
                     HistoryOf(covm, true);
                  }
               }
               catch (Exception)
               {
                  // Undo isn't worth stopping the editor for
               }
            }
         }), System.Windows.Threading.DispatcherPriority.Background);
      }

      // Makes any change to the edited object since the last checkpoint one
      // undo step. MainWindow calls this shortly after each mouse button
      // release, key release and mouse wheel turn.
      public void UndoCheckpoint()
      {
         if (_restoringUndo)
         {
            return;
         }

         try
         {
            UndoHistory h = HistoryOf(EditedCpVm, true);
            if (h != null)
            {
               h.Checkpoint(EditedCpVm.ModelObject);
            }
         }
         catch (Exception)
         {
            // E.g. an object being built while the checkpoint runs; the next
            // checkpoint will catch the change
         }
      }

      private void UndoRebaseline(CompoundObjectViewModel covm)
      {
         try
         {
            UndoHistory h = HistoryOf(covm, false);
            if (h != null)
            {
               h.Rebaseline(covm.ModelObject);
            }
         }
         catch (Exception)
         {
         }
      }

      void UndoExecute(Object parameter)
      {
         CompoundObjectViewModel covm = EditedCpVm;
         UndoHistory h = HistoryOf(covm, true);
         if (h == null)
         {
            return;
         }

         CompoundObject state = h.Undo(covm.ModelObject);
         if (state == null)
         {
            SystemSounds.Beep.Play();
            return;
         }

         ApplyUndoState(covm, h, state);
      }

      void RedoExecute(Object parameter)
      {
         CompoundObjectViewModel covm = EditedCpVm;
         UndoHistory h = HistoryOf(covm, true);
         if (h == null)
         {
            return;
         }

         CompoundObject state = h.Redo(covm.ModelObject);
         if (state == null)
         {
            SystemSounds.Beep.Play();
            return;
         }

         ApplyUndoState(covm, h, state);
      }

      bool CanUndoExecute(Object parameter)
      {
         // Changes not yet checkpointed can be undone too, so this is only
         // a hint for the menu
         UndoHistory h = HistoryOf(EditedCpVm, false);
         return (h != null) && h.CanUndo;
      }

      bool CanRedoExecute(Object parameter)
      {
         UndoHistory h = HistoryOf(EditedCpVm, false);
         return (h != null) && h.CanRedo;
      }

      public ICommand Undo
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => UndoExecute(parameter), parameter => CanUndoExecute(parameter));
         }
      }

      public ICommand Redo
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => RedoExecute(parameter), parameter => CanRedoExecute(parameter));
         }
      }

      // Puts a state from the history into the edited object, rebuilds what
      // is shown and selects again what was selected (by name).
      private void ApplyUndoState(CompoundObjectViewModel covm, UndoHistory h, CompoundObject state)
      {
         HashSet<string> selectedShapes = new HashSet<string>();
         foreach (LfShapeViewModel svm in SelectedShapes)
         {
            if (svm.Name != null)
            {
               selectedShapes.Add(svm.Name);
            }
         }

         HashSet<string> selectedChildren = new HashSet<string>();
         foreach (ChildObjectViewModel chvm in SelectedChildObjects)
         {
            if (chvm.Name != null)
            {
               selectedChildren.Add(chvm.Name);
            }
         }

         _LeftClickState = LeftClickState.none;
         _restoringUndo = true;

         BeginSelectionBatch();
         try
         {
            DeselectAll();

            covm.RestoreModelContents(state);
            covm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);
            covm.InvalidateJoints();
            covm.InvalidateChildObjects();

            foreach (object o in covm.ShapeCollection.Shapes)
            {
               LfShapeViewModel svm = o as LfShapeViewModel;
               if ((svm != null) && (svm.Name != null) && selectedShapes.Contains(svm.Name))
               {
                  svm.IsSelected = true;
               }
            }

            foreach (ChildObjectViewModel chvm in covm.ChildObjectCollection.Children)
            {
               if ((chvm.Name != null) && selectedChildren.Contains(chvm.Name))
               {
                  SetChildSelected(chvm, true);
               }
            }

            // The objects this one is inside of have new bounding boxes
            CompoundObjectViewModel p = covm.ParentVm;
            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p = p.ParentVm;
            }

            h.Rebaseline(covm.ModelObject);
         }
         finally
         {
            _restoringUndo = false;
            EndSelectionBatch();
         }

         UpdateEditArea();
      }

      #endregion

      #region Copy and paste

      private EditorClipboard _clipboard = new EditorClipboard();

      // Last mouse position over the edit area, in the coordinates of the
      // edited object (set by MainWindow). Null until the mouse has been
      // over the edit area.
      public Point? LastMousePoint { get; set; }

      void CopyExecute(Object parameter)
      {
         CompoundObjectViewModel covm = EditedCpVm;
         _clipboard.Clear();

         double minX = double.MaxValue, minY = double.MaxValue;
         double maxX = double.MinValue, maxY = double.MinValue;
         HashSet<string> copiedNames = new HashSet<string>();

         foreach (LfShapeViewModel svm in SelectedShapes)
         {
            if (svm.ModelObject == null)
            {
               continue;
            }

            _clipboard.Shapes.Add(EditorClipboard.Clone(svm.ModelObject));
            copiedNames.Add(svm.Name);
            minX = Math.Min(minX, svm.PosX);
            maxX = Math.Max(maxX, svm.PosX);
            minY = Math.Min(minY, svm.PosY);
            maxY = Math.Max(maxY, svm.PosY);
         }

         foreach (ChildObjectViewModel chvm in SelectedChildObjects)
         {
            _clipboard.Children.Add(EditorClipboard.CloneChild(chvm.ModelObject));
            minX = Math.Min(minX, chvm.PosX);
            maxX = Math.Max(maxX, chvm.PosX);
            minY = Math.Min(minY, chvm.PosY);
            maxY = Math.Max(maxY, chvm.PosY);
         }

         // Joints come along when all the bodies they join are copied
         // (a rope may be tied to one body only)
         if (covm != null)
         {
            foreach (object o in covm.JointCollection.Joints)
            {
               WeldJointViewModel jvm = o as WeldJointViewModel;
               if ((jvm == null) || (jvm.ModelObject == null))
               {
                  continue;
               }

               WeldJoint j = jvm.ModelObject;
               bool aCopied = copiedNames.Contains(j.AName);
               bool bCopied = copiedNames.Contains(j.BName);
               bool bUnused = string.IsNullOrEmpty(j.BName) || (j.BName == "notDefined");

               if (aCopied && (bCopied || ((j is Rope) && bUnused)))
               {
                  _clipboard.Joints.Add(EditorClipboard.Clone(j));
               }
            }
         }

         if (!_clipboard.IsEmpty)
         {
            _clipboard.CenterX = (minX + maxX) / 2;
            _clipboard.CenterY = (minY + maxY) / 2;
         }
      }

      bool CanCopyExecute(Object parameter)
      {
         return (EditedCpVm != null) && ((SelectedShapes.Count > 0) || (SelectedChildObjects.Count > 0));
      }

      public ICommand Copy
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => CopyExecute(parameter), parameter => CanCopyExecute(parameter));
         }
      }

      // Parameter "inPlace" keeps the copied positions; otherwise the copies
      // are centred on the mouse (or moved a little if the mouse hasn't been
      // over the edit area). Names are made unique in the edited object, and
      // pasted joints are connected to the pasted shapes.
      void PasteExecute(Object parameter)
      {
         CompoundObjectViewModel covm = EditedCpVm;
         if ((covm == null) || _clipboard.IsEmpty)
         {
            return;
         }

         double dx = 0, dy = 0;
         if ((parameter as string) != "inPlace")
         {
            if (LastMousePoint.HasValue)
            {
               dx = LastMousePoint.Value.X - _clipboard.CenterX;
               dy = LastMousePoint.Value.Y - _clipboard.CenterY;
            }
            else
            {
               dx = 10;
               dy = 10;
            }
         }

         HashSet<string> shapeNames = new HashSet<string>();
         foreach (object o in covm.ShapeCollection.Shapes)
         {
            if (o is LfShapeViewModel)
            {
               shapeNames.Add(((LfShapeViewModel)o).Name);
            }
         }

         HashSet<string> jointNames = new HashSet<string>();
         foreach (object o in covm.JointCollection.Joints)
         {
            if (o is WeldJointViewModel)
            {
               jointNames.Add(((WeldJointViewModel)o).Name);
            }
         }

         HashSet<string> childNames = new HashSet<string>();
         foreach (ChildObjectViewModel chvm in covm.ChildObjectCollection.Children)
         {
            childNames.Add(chvm.Name);
         }

         DeselectAll();

         List<TreeViewViewModel> pasted = new List<TreeViewViewModel>();
         Dictionary<string, string> renamed = new Dictionary<string, string>();

         foreach (LfShape original in _clipboard.Shapes)
         {
            LfShape shape = EditorClipboard.Clone(original);
            string newName = EditorClipboard.UniqueName(shape.Name, shapeNames);
            shapeNames.Add(newName);
            if ((shape.Name != null) && !renamed.ContainsKey(shape.Name))
            {
               renamed[shape.Name] = newName;
            }
            shape.Name = newName;
            shape.PosX += dx;
            shape.PosY += dy;

            LfShapeViewModel vm = covm.AddShapeModel(shape);
            if (vm != null)
            {
               pasted.Add(vm);
            }
         }

         foreach (WeldJoint original in _clipboard.Joints)
         {
            WeldJoint joint = EditorClipboard.Clone(original);
            if ((joint.AName != null) && renamed.ContainsKey(joint.AName))
            {
               joint.AName = renamed[joint.AName];
            }
            if ((joint.BName != null) && renamed.ContainsKey(joint.BName))
            {
               joint.BName = renamed[joint.BName];
            }
            joint.Name = EditorClipboard.UniqueName(joint.Name, jointNames);
            jointNames.Add(joint.Name);

            WeldJointViewModel vm = covm.AddJointModel(joint);
            if (vm != null)
            {
               pasted.Add(vm);
            }
         }

         foreach (ChildObject original in _clipboard.Children)
         {
            ChildObject child = EditorClipboard.CloneChild(original);
            child.Name = EditorClipboard.UniqueName(child.Name, childNames);
            childNames.Add(child.Name);
            child.PosX += dx;
            child.PosY += dy;

            pasted.Add(covm.AddChildObjectModel(child));
         }

         covm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);
         if (_clipboard.Joints.Count > 0)
         {
            covm.InvalidateJoints();
         }

         // Select what was pasted, so it can be moved straight away
         foreach (TreeViewViewModel vm in pasted)
         {
            vm.IsSelected = true;
         }

         UpdateEditArea();
      }

      bool CanPasteExecute(Object parameter)
      {
         return (EditedCpVm != null) && !_clipboard.IsEmpty;
      }

      public ICommand Paste
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => PasteExecute(parameter), parameter => CanPasteExecute(parameter));
         }
      }

      #endregion

      void LocateInBrowserExecute(Object parameter)
      {
         if (parameter is TreeViewViewModel)
         {
            TreeViewViewModel tvvm = parameter as TreeViewViewModel;
            TreeViewViewModel originalTvvm = tvvm;

            tvvm.IsSelected = false;

            List<TreeViewViewModel> expandBransch = new List<TreeViewViewModel>();

            while (tvvm.TreeParent != null)
            {
               expandBransch.Add(tvvm.TreeParent);
               tvvm = tvvm.TreeParent;
            }

            for (int i = expandBransch.Count - 1; i >= 0; i--)
            {
               expandBransch[i].IsExpanded = true;
            }

            ((MainWindow)System.Windows.Application.Current.MainWindow).TheTreeView.UpdateLayout();

            originalTvvm.IsSelected = true;
         }
      }

      bool CanLocateInBrowserExecute(Object parameter)
      {
         return true;
      }

      public ICommand LocateInBrowser
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => LocateInBrowserExecute(parameter), parameter => CanLocateInBrowserExecute(parameter));
         }
      }



      void DebugHaltExecute(Object parameter)
      {
         int a = 10;
      }

      bool CanDebugHaltExecute(Object parameter)
      {
         return true;
      }

      public ICommand DebugHalt
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => DebugHaltExecute(parameter), parameter => CanDebugHaltExecute(parameter));
         }
      }


      #endregion

      #region Public Methods

      // NOTE: FrameworkElement is used to communicate from the View. It can 
      // be used to obtain the ViewModel object from it's datacontext.
      // In some cases we need the type of FrameworkElement (to see if a 
      // Line or Rectangle).

      // All mouse handling function return true if the the data was used for some action
      // (if original event was handled)


      public bool MouseDown(
         MouseEventObjectType objectType, 
         object sender, 
         MouseButton button, 
         Point clickPoint, 
         int clickCount, 
         bool shift, 
         bool ctrl, 
         bool alt)
      {
         if (button == MouseButton.Left)
         {
            _movedSincePress = false;

            if (clickCount > 1)
            {
               if (_LeftClickState == LeftClickState.addPoint)
               {
                  TerminatePointAdding();
                  return true;
               }
            }

            // If we are creating shapes or adding points to a polygon
            // we want to be able to click anywhere (even in an existing shape).
            // Therefore, we return false for any of those cases and hopefully
            // the mouse event will bubble up to the background canvas
            // and thus be handeled in that mouse handler.
            if (_LeftClickState != LeftClickState.none)
            {
               return false;
            }
         }

         if ((button == MouseButton.Left) || (button == MouseButton.Right))
         {
            // Decode target ViewModel and view object that was clicked
            switch (objectType)
            {
               case MouseEventObjectType.shape:
                  // Pressing on a shape selects it (or the child object it is
                  // part of) straight away, so it can be dragged by its inside.
                  // If it already was selected, the selection stays as it is,
                  // so everything selected can be dragged together. With ctrl
                  // or shift the selection changes on release instead (see
                  // MainWindow: the press goes on to the rubber band).
                  if ((button == MouseButton.Left) && !ctrl && !shift && (sender is LfShapeViewModel))
                  {
                     PressSelect((LfShapeViewModel)sender);
                  }
                  return false;

               case MouseEventObjectType.gunSystem:
               case MouseEventObjectType.flameEmitterSystem:
               case MouseEventObjectType.objectFactory:
               case MouseEventObjectType.shieldSystem:
               case MouseEventObjectType.systemAnchorA:
               case MouseEventObjectType.systemAnchorB:
                  // So far we do not do anything here
                  return false;

               case MouseEventObjectType.dragableBorder:
               case MouseEventObjectType.rotatableObjectDragableBorder:
                  break;

               case MouseEventObjectType.dragablePoint:

                  LfDragablePointViewModel dpvm = (LfDragablePointViewModel)sender;

                  if (dpvm.IsSelected)
                  {
                     // Could be the beginning of dragging

                  }
                  else
                  {
                     if (!ctrl)
                     {
                        // We dont want to deselect absolutly everything here,
                        // only the selected points 
                        DeselectAllPoints();
                     }

                     dpvm.IsSelected = true;
                  }

                  return true;
              
               case MouseEventObjectType.compoundObjectBoundaryBox:
                  // So far we do not do anything here
                  // CompoundObjectViewModel covm = (CompoundObjectViewModel)sender;
                  return false;

               case MouseEventObjectType.none:
                  BackgroundMouseDown(clickPoint, button, clickCount, shift, ctrl, alt);
                  return false;
            }
         }

         return false;
      }

      public bool MouseMove(
         MouseEventObjectType objectType,
         object sender,
         Vector dragVector,
         bool shift,
         bool ctrl,
         bool alt)
      {
         // If we are creating shapes or adding points to a polygon
         // we want to be able to click anywhere (even in an existing shape).
         // Therefore, we return false for any of those cases and hopefully
         // the mouse event will bubble up to the background canvas
         // and thus be handeled in that mouse handler.
         if (_LeftClickState != LeftClickState.none)
         {
            return false;
         }

         Point vectPoint;
         Point rotPoint;
         Vector rotatedDragVector;

         // Something is being dragged (the views only call this once the
         // mouse has moved a few pixels with the button down)
         _movedSincePress = true;


         // Decode target ViewModel and view oobject that was clicked
         switch (objectType)
         {
            case MouseEventObjectType.shape:
            case MouseEventObjectType.dragableBorder:
            case MouseEventObjectType.dragablePolygonBorder:
            case MouseEventObjectType.compoundObjectBoundaryBox:

               // Dragging a shape by its inside or its outline, or a child
               // object by its frame, moves everything that is selected
               MoveSelection(dragVector);

               return true;

            case MouseEventObjectType.rotatableObjectDragableBorder:

               // Mouse move on rectangle of Rotatable object point
               LfPointViewModel pvm = (LfPointViewModel)sender;

               // Before moving all vertices, we rotate the vector to match the shape rotation.
               vectPoint = new Point(dragVector.X, dragVector.Y);
               rotPoint = CoordinateTransformations.LocalPointFromRotated(vectPoint, pvm.PointsParent.Angle);
               rotatedDragVector = new Vector(rotPoint.X, rotPoint.Y);

               foreach (CoSystemViewModel child in SelectedSystems)
               {
                  if (child.Properties is IPositionInterface)
                  {
                     IPositionInterface sysPos = child.Properties as IPositionInterface;
                     sysPos.PosX += rotatedDragVector.X;
                     sysPos.PosY += rotatedDragVector.Y;
                  }
               }


               return true;

            case MouseEventObjectType.dragablePoint:

               // Mouse move on rectangle of DragablePoint
               LfDragablePointViewModel dpvm = (LfDragablePointViewModel)sender;

               // Before moving all vertices, we rotate the vector to match the shape rotation.
               vectPoint = new Point(dragVector.X, dragVector.Y);
               rotPoint = CoordinateTransformations.LocalPointFromRotated(vectPoint, dpvm.Parent.Angle);
               rotatedDragVector = new Vector(rotPoint.X, rotPoint.Y);
               
               foreach (LfDragablePointViewModel point in _selectedPoints)
               {
                  point.PosX += rotatedDragVector.X;
                  point.PosY += rotatedDragVector.Y;
               }

               return true;

            case MouseEventObjectType.jointAnchorA:

               WeldJointViewModel wjvm = (WeldJointViewModel)sender;

               // Before moving, we rotate the vector to match the shape rotation.
               vectPoint = new Point(dragVector.X, dragVector.Y);
               rotPoint = CoordinateTransformations.LocalPointFromRotated(vectPoint, wjvm.AShapeObject.Angle);
               rotatedDragVector = new Vector(rotPoint.X, rotPoint.Y);

               // Find new point
               wjvm.AAnchorX += rotatedDragVector.X;
               wjvm.AAnchorY += rotatedDragVector.Y;

               wjvm.OnPropertyChanged("");

               return true;

            case MouseEventObjectType.jointAnchorB:

               wjvm = (WeldJointViewModel)sender;

               // Before moving, we rotate the vector to match the shape rotation.
               vectPoint = new Point(dragVector.X, dragVector.Y);
               rotPoint = CoordinateTransformations.LocalPointFromRotated(vectPoint, wjvm.BShapeObject.Angle);
               rotatedDragVector = new Vector(rotPoint.X, rotPoint.Y);

               // Find new point
               wjvm.BAnchorX += rotatedDragVector.X;
               wjvm.BAnchorY += rotatedDragVector.Y;

               wjvm.OnPropertyChanged("");

               return true;

            case MouseEventObjectType.prismJointUpperLimit:

               PrismaticJointViewModel pjvm = (PrismaticJointViewModel)sender;

               // Before moving, we rotate the vector to match the shape rotation.
               vectPoint = new Point(dragVector.X, dragVector.Y);
               rotPoint = CoordinateTransformations.LocalPointFromRotated(vectPoint, pjvm.AShapeObject.Angle);
               rotatedDragVector = new Vector(rotPoint.X, rotPoint.Y);

               // Find new point
               double newPosX = pjvm.UpperLimitPosX + rotatedDragVector.X;
               double newPosY = pjvm.UpperLimitPosY + rotatedDragVector.Y;

               // Now calculate direction vector and upper limit from point
               Vector v = new Vector(newPosX, newPosY);
               Vector f = new Vector(pjvm.AAnchorX, pjvm.AAnchorY);
               v = v - f;
               pjvm.UpperLimit = v.Length;

               if (pjvm.LowerLimit > pjvm.UpperLimit)
               {
                  pjvm.LowerLimit = pjvm.UpperLimit;
               }

               v.Normalize();
               pjvm.AAxisX = v.X;
               pjvm.AAxisY = v.Y;
               
               pjvm.OnPropertyChanged("");

               return true;

            case MouseEventObjectType.prismJointLowerLimit:

               pjvm = (PrismaticJointViewModel)sender;

               // Before moving, we rotate the vector to match the shape rotation.
               vectPoint = new Point(dragVector.X, dragVector.Y);
               rotPoint = CoordinateTransformations.LocalPointFromRotated(vectPoint, pjvm.AShapeObject.Angle);
               rotatedDragVector = new Vector(rotPoint.X, rotPoint.Y);

               // Find new point
               newPosX = pjvm.LowerLimitPosX + rotatedDragVector.X;
               newPosY = pjvm.LowerLimitPosY + rotatedDragVector.Y;

               // Now calculate direction vector and upper limit from point
               f = new Vector(pjvm.AAnchorX, pjvm.AAnchorY);
               Vector upperVector = new Vector(pjvm.UpperLimitPosX, pjvm.UpperLimitPosY) - f;
               v = new Vector(newPosX, newPosY) - f;

               upperVector.Normalize();

               pjvm.LowerLimit = Vector.Multiply(upperVector, v);

               if (pjvm.LowerLimit > pjvm.UpperLimit)
               {
                  pjvm.LowerLimit = pjvm.UpperLimit;
               }
               //v.Normalize();
               //pjvm.AAxisX = v.X;
               //pjvm.AAxisY = v.Y;

               pjvm.OnPropertyChanged("");

               return true;

            case MouseEventObjectType.gunSystem:
            case MouseEventObjectType.flameEmitterSystem:
            case MouseEventObjectType.shieldSystem:

               BodyOriginSystemViewModel bosvm = (BodyOriginSystemViewModel)sender;

               // Before moving, we rotate the vector to match the shape rotation.
               vectPoint = new Point(dragVector.X, dragVector.Y);
               rotPoint = CoordinateTransformations.LocalPointFromRotated(vectPoint, bosvm.BodyObject.Angle);
               rotatedDragVector = new Vector(rotPoint.X, rotPoint.Y);

               // Find new point
               bosvm.SystemOriginX += rotatedDragVector.X;
               bosvm.SystemOriginY += rotatedDragVector.Y;

               bosvm.OnPropertyChanged("");

               return true;

            case MouseEventObjectType.systemAnchorA:

               ReentryFlameEmitterPropertiesViewModel refevm = (ReentryFlameEmitterPropertiesViewModel)sender;

               // Before moving, we rotate the vector to match the shape rotation.
               vectPoint = new Point(dragVector.X, dragVector.Y);
               rotPoint = CoordinateTransformations.LocalPointFromRotated(vectPoint, refevm.BodyObject.Angle);
               rotatedDragVector = new Vector(rotPoint.X, rotPoint.Y);

               // Find new point
               refevm.EmitterLineStartX += rotatedDragVector.X;
               refevm.EmitterLineStartY += rotatedDragVector.Y;

               refevm.OnPropertyChanged("");

               return true;

            case MouseEventObjectType.systemAnchorB:

               refevm = (ReentryFlameEmitterPropertiesViewModel)sender;

               // Before moving, we rotate the vector to match the shape rotation.
               vectPoint = new Point(dragVector.X, dragVector.Y);
               rotPoint = CoordinateTransformations.LocalPointFromRotated(vectPoint, refevm.BodyObject.Angle);
               rotatedDragVector = new Vector(rotPoint.X, rotPoint.Y);

               // Find new point
               refevm.EmitterLineEndX += rotatedDragVector.X;
               refevm.EmitterLineEndY += rotatedDragVector.Y;

               refevm.OnPropertyChanged("");

               return true;

            case MouseEventObjectType.none:
               break;
         }

         return false;
      }

      // clickPoint will be in coordinates of the parent CompoundObject
      public bool MouseUp(
         MouseEventObjectType objectType,
         object sender,
         MouseButton button,
         Point clickPoint,
         int clickCount,
         bool shift,
         bool ctrl,
         bool alt)
      {
         if (button == MouseButton.Left)
         {
            // If we are creating shapes or adding points to a polygon
            // we want to be able to click anywhere (even in an existing shape).
            // Therefore, we return false for any of those cases and hopefully
            // the mouse event will bubble up to the background canvas
            // and thus be handeled in that mouse handler.
            if (_LeftClickState != LeftClickState.none)
            {
               return false;
            }

         }

         if ((button == MouseButton.Left) || (button == MouseButton.Right))
         { 
            // Decode target ViewModel and view oobject that was clicked
            switch (objectType)
            {
               case MouseEventObjectType.shape:

                  // Mouse up on Shape
                  LfShapeViewModel shvm = (LfShapeViewModel)sender;

                  // The clicked shape is selected if it belongs to the edited
                  // object; if it is part of a child object, that child object
                  // is selected instead (see SelectionTarget).

                  if (button == MouseButton.Right)
                  {
                     // Right click selects what was clicked (unless it already
                     // is) and lets the event go on to open the context menu
                     if (!IsTargetSelected(shvm))
                     {
                        if (!ctrl && !shift)
                        {
                           DeselectAll();
                        }

                        SetTargetSelected(shvm, true);
                     }

                     return false;
                  }

                  if (_movedSincePress)
                  {
                     // End of a drag: the selection stays
                     return true;
                  }

                  if (ctrl || shift)
                  {
                     // Add to or take away from the selection
                     SetTargetSelected(shvm, !IsTargetSelected(shvm));
                  }
                  else if (_pressTargetWasSelected)
                  {
                     // A click (not a drag) on something that was already
                     // selected selects just that
                     BeginSelectionBatch();
                     try
                     {
                        DeselectAll();
                        SetTargetSelected(shvm, true);
                     }
                     finally
                     {
                        EndSelectionBatch();
                     }
                  }

                  return true;

               case MouseEventObjectType.objectFactory:
               case MouseEventObjectType.gunSystem:
               case MouseEventObjectType.flameEmitterSystem:
               case MouseEventObjectType.shieldSystem:
               case MouseEventObjectType.systemAnchorA:
               case MouseEventObjectType.systemAnchorB:

                  // Mouse up on Shape
                  SystemViewModelBase ofpvm = (SystemViewModelBase)sender;

                  // If the clicked shape is children of the edited object, it should
                  // be selected (assessing the ctrl key in the process for multiple 
                  // selection)
                  // If it is not part of the Editable object, the parent should be selected
                  // also assessing ctrl-key

                  if (AmISelectable(ofpvm))
                  {
                     if (!ctrl)
                     {
                        DeselectAll();
                     }

                     ofpvm.SystemViewModel.IsSelected = true;
                  }
                  else
                  {
                     // The clicked shape is part of a child object, select it or add
                     // it to selected childs
                     if (!ctrl)
                     {
                        DeselectAll();
                     }

                     ofpvm.SystemViewModel.ParentVm.IsSelected = true;
                  }

                  // If we pressed right click to select, we want to the rest
                  // of the even handling to handle context menu, so we does
                  // not mark the event as handled if we right click
                  return (button != MouseButton.Right);

               case MouseEventObjectType.jointAnchorA:
               case MouseEventObjectType.jointAnchorB:

                  WeldJointViewModel jvm = (WeldJointViewModel)sender;

                  if (AmISelectable(jvm))
                  {
                     if (!ctrl)
                     {
                        DeselectAll();
                     }

                     jvm.IsSelected = true;
                  }

                  // If we pressed right click to select, we want to the rest
                  // of the even handling to handle context menu, so we does
                  // not mark the event as handled if we right click
                  return (button != MouseButton.Right);


               case MouseEventObjectType.dragablePolygonBorder:

                  if (ctrl)
                  {
                     LfDragablePointViewModel dpvm = (LfDragablePointViewModel)sender;

                     // What is the click-point in this case?
                     // It is said to be the closesed parenting canvas which
                     // should be in CompoundObject coordinates. Lets try to convert
                     // this into the rotated shape coordinates. 

                     Point coPoint = clickPoint;

                     Point unrotatedShapePoint = dpvm.Parent.ParentVm.CoPointInShape(clickPoint, dpvm.Parent);

                     Point rotShapePoint = CoordinateTransformations.LocalPointFromRotated(unrotatedShapePoint, dpvm.Parent.Angle);

                     LfDragablePointViewModel newPoint = dpvm.Parent.InsertPoint(rotShapePoint, dpvm);

                     // We dont want to deselect absolutly everything here,
                     // only the selected points 
                     DeselectAllPoints();

                     newPoint.IsSelected = true;
                  }

                  // If we pressed right click to select, we want to the rest
                  // of the even handling to handle context menu, so we does
                  // not mark the event as handled if we right click
                  return (button != MouseButton.Right);

               case MouseEventObjectType.dragableBorder:
               case MouseEventObjectType.rotatableObjectDragableBorder:

                  return true;

               case MouseEventObjectType.dragablePoint:
                  // So far nothing is done here
                  // LfDragablePointViewModel dpvm = (LfDragablePointViewModel)sender;

                  return true;

               case MouseEventObjectType.compoundObjectBoundaryBox:
                  // So far nothing is done here
                  // CompoundObjectViewModel covm = (CompoundObjectViewModel)sender;

                  return true;

               case MouseEventObjectType.none:
                  return BackgroundMouseUp(clickPoint, button, shift, ctrl, alt);
            }
         }

         return false;
      }

      public bool BackgroundMouseUp(Point clickPoint, MouseButton button, bool shift, bool ctrl, bool alt)
      {
         // The click is in edit area coordinates; (0, 0) of the edited
         // object is at the CenteredCanvas's origin.
         Point origin = ((MainWindow)System.Windows.Application.Current.MainWindow).ContentOrigin;

         clickPoint.Offset(-origin.X, -origin.Y);

         if (EditedCpVm == null) return false;

         if (button == MouseButton.Left)
         {
            //Debug.WriteLine("Clicked on background");
            if (_LeftClickState == LeftClickState.none)
            {
               DeselectAll();
            }
            else if ((_LeftClickState == LeftClickState.staticBox) ||
               (_LeftClickState == LeftClickState.dynamicBox) ||
               (_LeftClickState == LeftClickState.dynamicCircle) ||
               (_LeftClickState == LeftClickState.staticCircle) ||
               (_LeftClickState == LeftClickState.spriteBox) || 
               (_LeftClickState == LeftClickState.staticPolygon) ||
               (_LeftClickState == LeftClickState.dynamicPolygon) ||
               (_LeftClickState == LeftClickState.spritePolygon) ||
               (_LeftClickState == LeftClickState.staticBoxedSpritePolygon) ||
               (_LeftClickState == LeftClickState.dynamicBoxedSpritePolygon))
            {
               LfShapeViewModel newShapeVm = EditedCpVm.AddShape(_LeftClickState, clickPoint);

               if (newShapeVm != null)
               {
                  SelectedShapes.Add(newShapeVm);

                  newShapeVm.IsSelected = true;

                  foreach (LfDragablePointViewModel selpoint in _selectedPoints)
                  {
                     selpoint.IsSelected = false;
                  }
                  _selectedPoints.Clear();

                  EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);

                  if (newShapeVm is LfPolygonViewModel)
                  {
                     LfDragablePointViewModel newPoint = ((LfPolygonViewModel)newShapeVm).InsertPoint(new Point(0, 0), null);
                     _selectedPoints.Add(newPoint);
                     newPoint.IsSelected = true;
                     _LeftClickState = LeftClickState.addPoint;
                  }
                  else
                  {
                     _LeftClickState = LeftClickState.none;
                  }
               }
            }
            else if ((_LeftClickState == LeftClickState.weldJoint) ||
               (_LeftClickState == LeftClickState.revoluteJoint) ||
               (_LeftClickState == LeftClickState.prismaticJoint) ||
               (_LeftClickState == LeftClickState.rope))
            {
               string bName = "notDefined";

               CompoundObjectViewModel covm = EditedCpVm as CompoundObjectViewModel;

               if (SelectedShapes.Count > 1)
               {
                  bName = SelectedShapes[1].Name;

                  if (SelectedShapes[0].Name == SelectedShapes[1].Name)
                  {
                     MessageBox.Show("The selected shapes has the same name and thus a unambigous joint can not be created. Rename at least one of the shapes.", "Error Creating Joint", MessageBoxButton.OK, MessageBoxImage.Error);

                     _LeftClickState = LeftClickState.none;
                     return true;
                  }
               }

               WeldJointViewModel newJointVm = covm.AddJoint(
                  _LeftClickState, 
                  clickPoint, 
                  SelectedShapes[0].Name,
                  bName, 
                  SelectedShapes.Count);

               foreach (LfDragablePointViewModel selpoint in _selectedPoints)
               {
                  selpoint.IsSelected = false;
               }
               _selectedPoints.Clear();

               newJointVm.IsSelected = true;

               EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);

               _LeftClickState = LeftClickState.none;
            }
            else if ((_LeftClickState == LeftClickState.objectFactory) ||
               (_LeftClickState == LeftClickState.gun) ||
               (_LeftClickState == LeftClickState.flameEmitter) ||
               (_LeftClickState == LeftClickState.shield) ||
               (_LeftClickState == LeftClickState.reentryFlameEmitter))
            {
               string shapeName = "notDefined";

               CompoundObjectViewModel covm = EditedCpVm as CompoundObjectViewModel;

               if (SelectedShapes.Count > 0)
               {
                  shapeName = SelectedShapes[0].Name;
               }

               CoSystemViewModel newSysVm = covm.AddSystem(
                  _LeftClickState, 
                  clickPoint, 
                  shapeName,
                  SelectedShapes.Count);

               newSysVm.IsSelected = true;

               EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);

               _LeftClickState = LeftClickState.none;

            }
            else if (_LeftClickState == LeftClickState.addPoint)
            {
               // When adding points to new polygon we require that the
               // shape is the only one selected
               if ((SelectedShapes.Count == 1) && (SelectedShapes[0] is LfPolygonViewModel))
               {
                  LfDragablePointViewModel newPoint = EditedCpVm.AddPoint((LfPolygonViewModel)SelectedShapes[0], clickPoint);

                  while (_selectedPoints.Count > 0)
                  {
                     _selectedPoints[0].IsSelected = false;
                  }

                  _selectedPoints.Add(newPoint);
                  newPoint.IsSelected = true;
               }

            }

            return true;
         }

         return false;
      }

      public bool BackgroundMouseDown(Point clickPoint, MouseButton button, int clickCount, bool shift, bool ctrl, bool alt)
      {
         if (clickCount > 1)
         {

            if (button == MouseButton.Left)
            {
               if (_LeftClickState == LeftClickState.addPoint)
               {
                  _LeftClickState = LeftClickState.none;
                  return true;
               }
            }
         }

         return false;
      }

      public bool KeyDownHandler(KeyEventArgs e)
      {
         if (e.Key == Key.Delete)
         {
            if (CanDeleteExecute(null))
            {
               DeleteExecute(null);
            }
         }

         return false;
      }

      public void RotateSelectedShape(int delta, bool fine)
      {
         foreach (LfShapeViewModel svm in SelectedShapes)
         {
            if (!svm.EditorHidden)
            {
               svm.RotateShape(delta, fine);
            }
         }

         foreach (CoSystemViewModel syvm in SelectedSystems)
         {
            if (syvm.Properties is GunPropertiesViewModel)
            {
               GunPropertiesViewModel gpvm = syvm.Properties as GunPropertiesViewModel;

               double increment;

               if (fine)
               {
                  increment = (double)delta * 0.1;
               }
               else
               {
                  increment = (double)delta * 1;
               }

               gpvm.Angle += increment / 120;

            }

            if (syvm.Properties is FlameEmitterPropertiesViewModel)
            {
               FlameEmitterPropertiesViewModel gpvm = syvm.Properties as FlameEmitterPropertiesViewModel;

               double increment;

               if (fine)
               {
                  increment = (double)delta * 0.1;
               }
               else
               {
                  increment = (double)delta * 1;
               }

               gpvm.Angle += increment / 120;

            }

            if (syvm.Properties is ReentryFlameEmitterPropertiesViewModel)
            {
               ReentryFlameEmitterPropertiesViewModel gpvm = syvm.Properties as ReentryFlameEmitterPropertiesViewModel;

               double increment;

               if (fine)
               {
                  increment = (double)delta * 0.1;
               }
               else
               {
                  increment = (double)delta * 1;
               }

               gpvm.Angle += increment / 120;

            }

         }
      }

      public bool AmISelectable(IParentInterface me)
      {
         return (me.ParentVm == EditedCpVm);
      }

      //public int GetEditableCoBehaviourStateIndex()
      //{
      //   if (EditedCpVm == null)
      //   {
      //      return 0;
      //   }

      //   if (!(EditedCpVm is CompoundObjectViewModel))
      //   {
      //      return 0;
      //   }

      //   CompoundObjectViewModel covm = EditedCpVm as CompoundObjectViewModel;

      //   return covm.Behaviour.DisplayedStateIndex;

      //}

      //public StateViewModel GetEditableCoBehaviourState()
      //{
      //   if (EditedCpVm == null)
      //   {
      //      return null;
      //   }

      //   if (!(EditedCpVm is CompoundObjectViewModel))
      //   {
      //      return null;
      //   }

      //   CompoundObjectViewModel covm = EditedCpVm as CompoundObjectViewModel;

      //   return covm.Behaviour.States[GetEditableCoBehaviourStateIndex()];

      //}

      //public string GetEditableCoBehaviourStateName()
      //{
      //   StateViewModel svm = GetEditableCoBehaviourState();

      //   if (svm != null)
      //   {
      //      return svm.StateName;
      //   }

      //   return "#Error should not reach here#";
      //}

      //public int GetEditableCoBehaviourIndexOf(IParentInterface child, string state)
      //{
      //   FileCOViewModel fvm = GetFileViewModelOfChild(child);
      //   if ((fvm != null) && (fvm.Behaviour != null))
      //   {
      //      CompoundObjectViewModel covm = EditedCpVm as CompoundObjectViewModel;
      //      return fvm.Behaviour.States.IndexOf(covm.Behaviour.FindStateVM(state));
      //   }

      //   return 0;
      //}

      public FileCOViewModel GetFileViewModelOfChild(IParentInterface child)
      {
         while ((child != null) && (!(child is FileCOViewModel)))
         {
            child = child.ParentVm;
         }

         return child as FileCOViewModel;
      }

      public void BuildSelectionCollections()
      {
         // Here we assess the current selection and updates 
         // the selection collection of different types

         // While many objects are selected or deselected at once, this is
         // done once at the end (EndSelectionBatch)
         if (_selectionBatch > 0)
         {
            _selectionBatchDirty = true;
            return;
         }

         if (EditedCpVm == null)
         {
            return;
         }

         SelectedChildObjects.Clear();
         SelectedShapes.Clear();
         SelectedJoints.Clear();
         SelectedSystems.Clear();
         SelectedPoints.Clear();
         EditableSpawnObject = null;

         foreach (object o in EditedCpVm.ShapeCollection.Shapes)
         {
            if (o is LfShapeViewModel)
            {
               LfShapeViewModel shape = o as LfShapeViewModel;

               if (AmISelectable(shape))
               {
                  // This is the shape of the object being edited, 
                  if (shape.IsSelected)
                  {
                     SelectedShapes.Add(shape);
                  }

                  if (shape is LfPolygonViewModel)
                  {
                     LfPolygonViewModel polygon = shape as LfPolygonViewModel;

                     foreach (LfDragablePointViewModel dp in polygon.PointVms)
                     {
                        if (dp.IsSelected)
                        {
                           // This is the shape of the object being edited, 
                           SelectedPoints.Add(dp);
                        }
                     }
                  }
               }
            }
         }

         if (EditedCpVm is CompoundObjectViewModel)
         {
            CompoundObjectViewModel covm = EditedCpVm as CompoundObjectViewModel;

            foreach (TreeViewViewModel tvvm in covm.ChildObjectCollection.Children)
            {
               if (tvvm is ChildObjectViewModel)
               {
                  ChildObjectViewModel chvm = tvvm as ChildObjectViewModel;

                  if (AmISelectable(chvm))
                  {
                     // This is the child object of the object being edited, 
                     if (chvm.IsSelected)
                     {
                        SelectedChildObjects.Add(chvm);
                     }

                     //foreach (ChildCOViewModel chcvm in chvm.StateProperties)
                     //{
                     //   if (chcvm is ChildCOViewModel)
                     //   {
                     //      ChildCOViewModel cospvm = chcvm as ChildCOViewModel;

                     //      if (cospvm.IsSelected)
                     //      {
                     //         SelectedChildObjectStateProperties.Add(cospvm);
                     //      }
                     //   }
                     //}
                  }
               }
            }

            foreach (object o in covm.JointCollection.Joints)
            {
               if (o is WeldJointViewModel)
               {
                  WeldJointViewModel joint = o as WeldJointViewModel;

                  if (AmISelectable(joint))
                  {
                     if (joint.IsSelected)
                     {
                        // This is the shape of the object being edited, 
                        SelectedJoints.Add(joint);
                     }
                  }
               }
            }

            foreach (object o in covm.SystemCollection.Systems)
            {
               if (o is CoSystemViewModel)
               {
                  CoSystemViewModel system = o as CoSystemViewModel;

                  if (AmISelectable(system))
                  {
                     if (system.IsSelected)
                     {
                        // This is the shape of the object being edited, 
                        SelectedSystems.Add(system);
                     }

                     // Check if any spawn objects are selected in this system
                     // and set EditableSpawnObject to it.
                     // Note that this may be overwritten when spawn obejcts
                     // are hunted in the BrakableObject behaviour.
                     if (system.Type == "objectFactory")
                     {
                        ObjectFactoryPropertiesViewModel ofvm = system.Properties as ObjectFactoryPropertiesViewModel;

                        foreach (SpawnObjectViewModel sovm in ofvm.SpawnObjects)
                        {
                           if (sovm.IsSelected)
                           {
                              EditableSpawnObject = sovm;
                           }

                           TreeViewViewModel tvvm = sovm.SpawnChildObject[0];

                           if (tvvm is ChildObjectViewModel)
                           {
                              ChildObjectViewModel chvm = tvvm as ChildObjectViewModel;

                              if (AmISelectable(chvm))
                              {
                                 // This is the child object of the object being edited, 
                                 if (covm.IsSelected)
                                 {
                                    SelectedChildObjects.Add(chvm);
                                 }

                                 //foreach (ChildObjectViewModel chcvm in chvm.Chil)
                                 //{
                                 //   if (chcvm is ChildObjectViewModel)
                                 //   {
                                 //      ChildObjectViewModel cospvm = chcvm as ChildObjectViewModel;

                                 //      if (cospvm.IsSelected)
                                 //      {
                                 //         SelectedChildObjects.Add(cospvm);
                                 //      }
                                 //   }
                                 //}
                              }
                           }
                        }
                     }
                  }
               }
            }

            // Now assess if the spawn object is selected.
            // There can, potentially, multiple selected spawn object
            // but it would not make sense, so let the last selected
            // spawn object be the one to be edited.
            // Also, spawn object can exists in breakable objects
            // and object factories
            if (covm.Behaviour.BehaviourProperties is BreakableObjectPropertiesViewModel)
            {
               BreakableObjectPropertiesViewModel breakProp = covm.Behaviour.BehaviourProperties as BreakableObjectPropertiesViewModel;

               foreach (SpawnObjectViewModel sovm in breakProp.SpawnObjects)
               {
                  if (sovm.IsSelected)
                  {
                     EditableSpawnObject = sovm;
                  }

                  TreeViewViewModel tvvm = sovm.SpawnChildObject[0];

                  if (tvvm is ChildObjectViewModel)
                  {
                     ChildObjectViewModel chvm = tvvm as ChildObjectViewModel;

                     if (AmISelectable(chvm))
                     {
                        // This is the child object of the object being edited, 
                        if (covm.IsSelected)
                        {
                           SelectedChildObjects.Add(chvm);
                        }

                        //foreach (ChildCOViewModel chcvm in chvm.StateProperties)
                        //{
                        //   if (chcvm is ChildCOViewModel)
                        //   {
                        //      ChildCOViewModel cospvm = chcvm as ChildCOViewModel;

                        //      if (cospvm.IsSelected)
                        //      {
                        //         SelectedChildObjectStateProperties.Add(cospvm);
                        //      }
                        //   }
                        //}
                     }
                  }
               }
            }
         }
      }

      #endregion

      #region private Methods

      private void TerminatePointAdding()
      {
         SystemSounds.Beep.Play();

         _LeftClickState = LeftClickState.none;

         // Lets do a brute force invalidate of all points of the polygon
         if ((SelectedShapes.Count() == 1) && (SelectedShapes[0] is LfPolygonViewModel))
         {
            LfPolygonViewModel pvm = (LfPolygonViewModel)SelectedShapes[0];

            foreach (LfDragablePointViewModel dpvm in pvm.PointVms)
            {
               dpvm.OnPropertyChanged("");
            }
         }

      }

      //private void DeselectJoints()
      //{
      //   foreach (WeldJointViewModel selJoint in SelectedJoints)
      //   {
      //      selJoint.IsSelected = false;
      //   }
      //   SelectedJoints.Clear();
      //}

      //private void DeselectShapes()
      //{
      //   foreach (WeldJointViewModel selJoint in SelectedJoints)
      //   {
      //      selJoint.IsSelected = false;
      //   }
      //   SelectedJoints.Clear();
      //}

      private void DeselectAll()
      {
         //((MainWindow)System.Windows.Application.Current.MainWindow).TheTreeView.ClearSelection();

         BeginSelectionBatch();
         try
         {
            foreach (FileCOViewModel coFile in FileCollectionViewModel)
            {
               coFile.IsSelected = false;
               coFile.DeselectAllChildren();
            }
         }
         finally
         {
            EndSelectionBatch();
         }
      }

      #region Selection

      // Every change of an object's IsSelected rebuilds the selection
      // collections (and with them the properties panel). When many change
      // at once, that is done once, at the end.
      private int _selectionBatch = 0;
      private bool _selectionBatchDirty = false;

      // Set when the mouse has dragged something since the left button went
      // down (a release is then the end of a drag, not a click)
      private bool _movedSincePress = false;

      // Whether what was pressed on was selected before the press
      private bool _pressTargetWasSelected = false;

      public void BeginSelectionBatch()
      {
         _selectionBatch++;
      }

      public void EndSelectionBatch()
      {
         if (_selectionBatch > 0)
         {
            _selectionBatch--;
         }

         if ((_selectionBatch == 0) && _selectionBatchDirty)
         {
            _selectionBatchDirty = false;
            BuildSelectionCollections();
         }
      }

      // The child object of the edited object that a shape is part of (also
      // when it is in a child of a child), or null if the shape isn't in a
      // child object of the edited object.
      private ChildObjectViewModel EditedChildOf(LfShapeViewModel shvm)
      {
         CompoundObjectViewModel covm = shvm.ParentVm;

         while (covm != null)
         {
            ChildObjectViewModel chvm = covm.TreeParent as ChildObjectViewModel;

            if (chvm == null)
            {
               return null;
            }

            if (chvm.ParentVm == EditedCpVm)
            {
               return chvm;
            }

            covm = chvm.ParentVm;
         }

         return null;
      }

      // What clicking a shape selects: the shape if it belongs to the edited
      // object, otherwise the child object it is part of.
      private bool IsTargetSelected(LfShapeViewModel shvm)
      {
         if (AmISelectable(shvm))
         {
            return shvm.IsSelected;
         }

         ChildObjectViewModel chvm = EditedChildOf(shvm);
         if (chvm != null)
         {
            return chvm.IsSelected;
         }

         return (shvm.ParentVm != null) && shvm.ParentVm.IsSelected;
      }

      private void SetTargetSelected(LfShapeViewModel shvm, bool selected)
      {
         if (AmISelectable(shvm))
         {
            shvm.IsSelected = selected;
            return;
         }

         ChildObjectViewModel chvm = EditedChildOf(shvm);
         if (chvm != null)
         {
            SetChildSelected(chvm, selected);
            return;
         }

         // Not in the edited object (e.g. an imported file): as before,
         // select the object the shape belongs to
         if (shvm.ParentVm != null)
         {
            shvm.ParentVm.IsSelected = selected;
         }
      }

      // A child object is shown as selected by its ChildCOViewModel (the
      // frame) and is moved through its ChildObjectViewModel
      private void SetChildSelected(ChildObjectViewModel chvm, bool selected)
      {
         BeginSelectionBatch();
         try
         {
            chvm.IsSelected = selected;
            if (chvm.CompObj != null)
            {
               chvm.CompObj.IsSelected = selected;
            }
         }
         finally
         {
            EndSelectionBatch();
         }
      }

      // Left button pressed on a shape (no ctrl or shift): select what it
      // stands for unless that already is selected
      private void PressSelect(LfShapeViewModel shvm)
      {
         _pressTargetWasSelected = IsTargetSelected(shvm);

         if (!_pressTargetWasSelected)
         {
            BeginSelectionBatch();
            try
            {
               DeselectAll();
               SetTargetSelected(shvm, true);
            }
            finally
            {
               EndSelectionBatch();
            }
         }
      }

      // Ctrl or shift click on a shape: add it to the selection or take it
      // away
      public void ToggleSelection(LfShapeViewModel shvm)
      {
         if ((shvm == null) || (EditedCpVm == null))
         {
            return;
         }

         SetTargetSelected(shvm, !IsTargetSelected(shvm));
      }

      // Rubber band: select the shapes and child objects of the edited object
      // that lie completely inside the rectangle (in the edited object's
      // coordinates). With add, they are added to the selection; otherwise
      // they replace it.
      public void SelectInRect(Rect r, bool add)
      {
         if (EditedCpVm == null)
         {
            return;
         }

         BeginSelectionBatch();
         try
         {
            if (!add)
            {
               DeselectAll();
            }

            foreach (object o in EditedCpVm.ShapeCollection.Shapes)
            {
               LfShapeViewModel svm = o as LfShapeViewModel;
               if ((svm == null) || svm.EditorHidden)
               {
                  continue;
               }

               Rect b = svm.BoundingBox;
               b.Offset(new Vector(svm.PosX, svm.PosY));
               if (!b.IsEmpty && r.Contains(b))
               {
                  svm.IsSelected = true;
               }
            }

            foreach (ChildObjectViewModel chvm in EditedCpVm.ChildObjectCollection.Children)
            {
               if (chvm.CompObj == null)
               {
                  continue;
               }

               Rect b = chvm.CompObj.BoundingBox;
               b.Offset(new Vector(chvm.PosX, chvm.PosY));
               if (!b.IsEmpty && r.Contains(b))
               {
                  SetChildSelected(chvm, true);
               }
            }
         }
         finally
         {
            EndSelectionBatch();
         }
      }

      // Everything a drag moves: selected shapes, child objects and systems
      private void MoveSelection(Vector dragVector)
      {
         foreach (LfShapeViewModel shape in SelectedShapes)
         {
            // A hidden shape that is still selected (from the tree view)
            // isn't dragged along with what can be seen
            if (shape.EditorHidden)
            {
               continue;
            }

            shape.PosX += dragVector.X;
            shape.PosY += dragVector.Y;
         }

         foreach (IPositionInterface child in SelectedChildObjects)
         {
            child.PosX += dragVector.X;
            child.PosY += dragVector.Y;
         }

         foreach (CoSystemViewModel child in SelectedSystems)
         {
            if (child.Properties is IPositionInterface)
            {
               IPositionInterface sysPos = child.Properties as IPositionInterface;
               sysPos.PosX += dragVector.X;
               sysPos.PosY += dragVector.Y;
            }
         }
      }

      void SelectAllExecute(Object parameter)
      {
         if (EditedCpVm == null)
         {
            return;
         }

         BeginSelectionBatch();
         try
         {
            DeselectAll();

            foreach (object o in EditedCpVm.ShapeCollection.Shapes)
            {
               LfShapeViewModel svm = o as LfShapeViewModel;
               if ((svm != null) && !svm.EditorHidden)
               {
                  svm.IsSelected = true;
               }
            }

            foreach (ChildObjectViewModel chvm in EditedCpVm.ChildObjectCollection.Children)
            {
               SetChildSelected(chvm, true);
            }
         }
         finally
         {
            EndSelectionBatch();
         }
      }

      // Hide in Editor for all selected shapes. They are deselected, so the
      // next drag doesn't move something that can't be seen.
      void HideSelectedExecute(Object parameter)
      {
         List<LfShapeViewModel> shapes = new List<LfShapeViewModel>(SelectedShapes);

         BeginSelectionBatch();
         try
         {
            foreach (LfShapeViewModel svm in shapes)
            {
               svm.EditorHidden = true;
               svm.IsSelected = false;
            }
         }
         finally
         {
            EndSelectionBatch();
         }
      }

      public ICommand HideSelected
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => HideSelectedExecute(parameter), parameter => (EditedCpVm != null) && (SelectedShapes.Count > 0));
         }
      }

      // Shows every hidden shape of the edited object again
      void UnhideAllExecute(Object parameter)
      {
         if (EditedCpVm == null)
         {
            return;
         }

         foreach (object o in EditedCpVm.ShapeCollection.Shapes)
         {
            LfShapeViewModel svm = o as LfShapeViewModel;
            if (svm != null)
            {
               svm.EditorHidden = false;
            }
         }
      }

      public ICommand UnhideAll
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => UnhideAllExecute(parameter), parameter => EditedCpVm != null);
         }
      }

      public ICommand SelectAll
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => SelectAllExecute(parameter), parameter => (EditedCpVm != null) && (_LeftClickState == LeftClickState.none));
         }
      }

      #endregion

      private void DeselectAllPoints()
      {
         // Loop like this since deselecting points will alter
         // the collection.
         for (int i = SelectedPoints.Count - 1; i >= 0; i--)
         {
            if (SelectedPoints[i] is LfDragablePointViewModel)
            {
               LfDragablePointViewModel ptvm = SelectedPoints[i] as LfDragablePointViewModel;

               ptvm.IsSelected = false;
            }
         }
      }

      private FileCOViewModel FindOpenedFile(string fileName)
      {
         foreach (FileCOViewModel fcovm in FileCollectionViewModel)
         {
            if (fcovm.Name == fileName)
            {
               // File is already opened, return with it
               return fcovm;
            }
         }

         return null;
      }

      private void UpdateFileReferences(FileCOViewModel updateReferencesToMe)
      {
         // Iterate all files except the file that is to be updated
         for (int i = 0; i < FileCollectionViewModel.Count; i++)
         {
            FileCOViewModel fcovm = FileCollectionViewModel[i];

            if (fcovm != updateReferencesToMe)
            {
               if (fcovm.ChildHasFileReference((updateReferencesToMe.FileName)))
               {
                  // If any child within this file has a reference to the 
                  // supplied object, we relaod the whole file
                  FileCollectionViewModel[i] = OpenFile(FileCollectionViewModel[i].FileName);
                  FileCollectionViewModel[i].BuildViewModel();

               }
            }
         }
      }

      private FileCOViewModel OpenFile(string fileName)
      {
         string s = GlobalConstants.DataDirectory + fileName;
         string fullPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
         string fullFileName = System.IO.Path.Combine(fullPath, s);

         ChildObject newChildObject = new ChildObject();

         CompoundObject newCP = CompoundObject.ReadFromFile(fullFileName);
         newChildObject.CompObj = newCP;
         newChildObject.Name = fileName;

         FileCOViewModel newCpVm = new FileCOViewModel(null, null, this, fileName, newCP);

         return newCpVm;

      }

      private void OpenFileToEdit(string fileName, bool exclusiveEdit)
      {
         // Lets first look to see if this file is not already openend
         FileCOViewModel fcovm = FindOpenedFile(fileName);

         if (fcovm != null)
         {
            // If file is already opened, we simply edit it
            EditedCpVm = fcovm;
         }
         else
         {
            FileCOViewModel newCpVm = OpenFile(fileName);

            FileCollectionViewModel.Add(newCpVm);

            EditedCpVm = newCpVm;
         }

         (EditedCpVm as CompoundObjectViewModel).BuildViewModel();
         EditedCpVm.BuildGlobalShapeCollection(ShowJoints, ShowSystems, ShowBackgrounds);

         EditedCpVm.OnPropertyChanged("");
         OnPropertyChanged("");

         Rect bib = EditedCpVm.BoundingBox;
         ((MainWindow)System.Windows.Application.Current.MainWindow).ShowThisRect(bib);

      }

      #endregion
   }
}
