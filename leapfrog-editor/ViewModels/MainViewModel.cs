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
      private ObservableCollection<ChildCOViewModel> _selectedChildObjectStateProperties = new ObservableCollection<ChildCOViewModel>();
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


         // Build collections of texture names
         // Process the list of files found in the directory.
         s = GlobalConstants.DataDirectory + "images";

         string[] fileEntries = Directory.GetFiles(s);
         foreach (string file in fileEntries)
         {
            string fname = System.IO.Path.GetFileNameWithoutExtension(file);
            string ext = System.IO.Path.GetExtension(file);

            if (ext == ".png")
            {
               Textures.Add(fname);
            }
         }
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
            _editedCpVm = value;
            OnPropertyChanged("EditedCpVm");
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

      public ObservableCollection<ChildCOViewModel> SelectedChildObjectStateProperties
      {
         get { return _selectedChildObjectStateProperties; }
         set { _selectedChildObjectStateProperties = value; }
      }

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
            EditedCpVm.BuildGlobalShapeCollection(GetEditableCoBehaviourStateName(), ShowJoints, ShowSystems, ShowBackgrounds);
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
            EditedCpVm.BuildGlobalShapeCollection(GetEditableCoBehaviourStateName(), ShowJoints, ShowSystems, ShowBackgrounds);
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
            EditedCpVm.BuildGlobalShapeCollection(GetEditableCoBehaviourStateName(), ShowJoints, ShowSystems, ShowBackgrounds);
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
            EditedCpVm.BuildGlobalShapeCollection(GetEditableCoBehaviourStateName(), ShowJoints, ShowSystems, ShowBackgrounds);
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

               ChildObjectStateProperties cosp = new ChildObjectStateProperties();
               cosp.File = fullFileName;

               TStateProperties<ChildObjectStateProperties>  newStateProp = new TStateProperties<ChildObjectStateProperties>();
               newStateProp.Properties = cosp;

               CompoundObject newCP = new CompoundObject();
               newStateProp.Properties.CompObj = newCP;

               ChildObject newChildObject = new ChildObject();
               newChildObject.StateProperties.Add(newStateProp);

               FileCOViewModel newCpVm = new FileCOViewModel(null, null, this, fileName, newCP);

               newCpVm.BuildViewModel();

               FileCollectionViewModel.Add(newCpVm);

               EditedCpVm = newCpVm;
               EditedCpVm.OnPropertyChanged("");
               OnPropertyChanged("");


               // Build global shape collection
               EditedCpVm.BuildGlobalShapeCollection(GetEditableCoBehaviourStateName(), ShowJoints, ShowSystems, ShowBackgrounds);

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
            EditedCpVm = parameter as CompoundObjectViewModel;
            EditedCpVm.OnPropertyChanged("");
            OnPropertyChanged("");
            DeselectAll();

            // Build global shape collection
            EditedCpVm.BuildGlobalShapeCollection(GetEditableCoBehaviourStateName(), ShowJoints, ShowSystems, ShowBackgrounds);

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

            //if ((covm.ModelObjectProperties.File == "") || (covm.ModelObjectProperties.File == "undef_file.xml"))
            //{
            //   return true;
            //}
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

            if (covm == EditedCpVm)
            {
               // We dont want to reopen the file that is already open
               // TODO: We need to iterate the file list to know if this 
               // file is open
               return false;
            }

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

            // Generate Triangles before saving
            fcovm.GenerateTriangles();

            fcovm.ModelObject.WriteToFile(fcovm.FullPathFileName);

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

            fvm.GenerateTriangles();

            fvm.ModelObject.WriteToFile(fvm.FullPathFileName);

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
               // Generate Triangles before saving
               fvm.GenerateTriangles();
               fvm.FileName = sfd.SafeFileName;
               fvm.ModelObject.WriteToFile(fvm.FullPathFileName);
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

            // Fortsätt här!!!!!

            //ChildObjectStateProperties childProps = new ChildObjectStateProperties();

            //childProps.File = fullFileName;

            //TStateProperties<ChildObjectStateProperties> newStateProp = new TStateProperties<ChildObjectStateProperties>();
            //newStateProp.State = "default";
            //newStateProp.Properties = childProps;

            //CompoundObject newCp = CompoundObject.ReadFromFile(fullFileName);

            //newStateProp.Properties.CompObj = newCp;

            //ChildObject newChildObject = new ChildObject();

            //newChildObject.StateProperties.Add(newStateProp);

            //CompoundObjectViewModel newCpVm = new CompoundObjectViewModel(null, null, this, newCp /*, newStateProp.Properties, newChildObject*/);
            //newCpVm.BuildViewModel(newChildObject);

            //EditedCpVm.ModelObject.ChildObjects.Add(newChildObject);
            //EditedCpVm.StateChildObjects.Children.Add(newCpVm);
            //OnPropertyChanged("");
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

            using (var client = new WebClient())
            {
               client.Credentials = new NetworkCredential("fredrikhoffman.se", "jessica21h");
               client.UploadFile("ftp://ftp.fredrikhoffman.se/leapfrog/updated_scene.xml", WebRequestMethods.Ftp.UploadFile, "updated_scene.xml");
            }

         }
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

      void DisplayStateExecute(Object parameter)
      {
         if (parameter is StateViewModel)
         {
            StateViewModel svm = parameter as StateViewModel;

            svm.SetOnDisplay();

            EditedCpVm.OnPropertyChanged("");
            EditedCpVm.BuildGlobalShapeCollection(svm.StateName, ShowJoints, ShowSystems, ShowBackgrounds);
         }
      }

      bool CanDisplayStateExecute(Object parameter)
      {
         if (parameter is StateViewModel)
         {
            StateViewModel svm = parameter as StateViewModel;

            if (!svm.IsOnDisplay)
            {
               return true;
            }
         }

         return false;
      }

      public ICommand DisplayState
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => DisplayStateExecute(parameter), parameter => CanDisplayStateExecute(parameter));
         }
      }

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
            TStateProperties<ChildObjectStateProperties> sp = new TStateProperties<ChildObjectStateProperties>();
            cho.StateProperties.Add(sp);

            ChildObjectStateProperties cosp = new ChildObjectStateProperties();
            sp.Properties = cosp;

            CompoundObject co = new CompoundObject();
            cosp.CompObj = co;

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

            EditedCpVm.BuildGlobalShapeCollection(GetEditableCoBehaviourStateName(), ShowJoints, ShowSystems, ShowBackgrounds);
         }
         else if (SelectedShapes.Count > 0)
         {
            foreach (LfShapeViewModel svm in SelectedShapes)
            {
               CompoundObjectViewModel covm = svm.ParentVm;

               //covm.ModelObject.RemoveShape(svm.ModelObject);
               covm.RemoveShape(svm);

               EditedCpVm.BuildGlobalShapeCollection(GetEditableCoBehaviourStateName(), ShowJoints, ShowSystems, ShowBackgrounds);
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

         if (parameter is ChildCOViewModel)
         {
            ChildCOViewModel chvm = parameter as ChildCOViewModel;
            ChildObjectViewModel covm = chvm.TreeParent as ChildObjectViewModel;

            covm.ModelObject.StateProperties.Remove(chvm.ChildStateModelObject);
            covm.StateProperties.Remove(chvm);
         }

         if (parameter is ChildObjectViewModel)
         {
            ChildObjectViewModel chvm = parameter as ChildObjectViewModel;
            CompoundObjectViewModel covm = chvm.ParentVm as CompoundObjectViewModel;

            covm.ModelObject.ChildObjects.Remove(chvm.ModelObject);
            covm.ChildObjectsWithStates.Children.Remove(chvm);
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

         EditedCpVm.BuildGlobalShapeCollection(GetEditableCoBehaviourStateName(), ShowJoints, ShowSystems, ShowBackgrounds);
      }

      bool CanDeleteThisObjectExecute(Object parameter)
      {
         if (parameter == null)
         {
            return false;
         }

         if ((EditedCpVm != null) && ((parameter is LfShapeViewModel) || 
            (parameter is WeldJointViewModel) || (parameter is ChildCOViewModel) ||
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
         if (parameter is ChildCollectionViewModel)
         {
            ChildCollectionViewModel scvm = parameter as ChildCollectionViewModel;

            ChildObject ch = new ChildObject();
            ChildObjectViewModel chvm = new ChildObjectViewModel(scvm, EditedCpVm as CompoundObjectViewModel, this, ch, true);

            CompoundObjectViewModel covm = EditedCpVm as CompoundObjectViewModel;
            covm.ModelObject.ChildObjects.Add(ch);
            scvm.Children.Add(chvm);
            EditedCpVm.BuildGlobalShapeCollection(GetEditableCoBehaviourStateName(), ShowJoints, ShowSystems, ShowBackgrounds);
         }

         if (parameter is ChildObjectViewModel)
         {
            ChildObjectViewModel chvm = parameter as ChildObjectViewModel;

            TStateProperties<ChildObjectStateProperties> sp = new TStateProperties<ChildObjectStateProperties>();
            chvm.ModelObject.StateProperties.Add(sp);

            sp.Properties = new ChildObjectStateProperties();
            sp.Properties.CompObj = new CompoundObject();

            ChildCOViewModel covm = new ChildCOViewModel(chvm, EditedCpVm as CompoundObjectViewModel, this, sp);
            covm.BuildViewModel(true);

            chvm.StateProperties.Add(covm);
            EditedCpVm.BuildGlobalShapeCollection(GetEditableCoBehaviourStateName(), ShowJoints, ShowSystems, ShowBackgrounds);
         }
      }

      bool CanAddChildObjectExecute(Object parameter)
      {
         if (parameter == null)
         {
            return false;
         }

         if ((EditedCpVm != null) && (EditedCpVm is CompoundObjectViewModel) && ((parameter is ChildCollectionViewModel) || (parameter is ChildObjectViewModel)))
         {
            return true;
         }

         return false;
      }

      public ICommand AddChildObject
      {
         get
         {
            return new MicroMvvm.RelayCommand<Object>(parameter => AddChildObjectExecute(parameter), parameter => CanAddChildObjectExecute(parameter));
         }
      }

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
                  // So far we do not do anything here
                  //LfShapeViewModel shvm = (LfShapeViewModel)sender;
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


         // Decode target ViewModel and view oobject that was clicked
         switch (objectType)
         {
            case MouseEventObjectType.shape:
               return false;

            case MouseEventObjectType.dragableBorder:
            case MouseEventObjectType.dragablePolygonBorder:
            case MouseEventObjectType.compoundObjectBoundaryBox:

               foreach (IPositionInterface shape in SelectedShapes)
               {
                  shape.PosX += dragVector.X;
                  shape.PosY += dragVector.Y;
               }

               foreach (IPositionInterface child in SelectedChildObjectStateProperties)
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

                  // If the clicked shape is children of the edited object but is not 
                  // the tree-child of a Parallaxbackground, it should
                  // be selected (assessing the ctrl key in the process for multiple 
                  // selection)
                  // If it is not part of the Editable object, the parent should be selected
                  // also assessing ctrl-key
                  // If it is the child of the Editable object but is also 
                  // the tree-child of a ParallaxBackgroundViewModel, the ParallaxBackground 
                  // should be selected in the tree view.

                  if (AmISelectable(shvm))
                  {
                     if (!ctrl)
                     {
                        DeselectAll();
                     }

                     shvm.IsSelected = true;
                  }
                  else
                  {
                     // The clicked shape is part of a child object, select it or add
                     // it to selected childs
                     if (!ctrl)
                     {
                        DeselectAll();
                     }

                     shvm.ParentVm.IsSelected = true;
                  }

                  // If we pressed right click to select, we want to the rest
                  // of the even handling to handle context menu, so we does
                  // not mark the event as handled if we right click
                  return (button != MouseButton.Right);

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
         // Shift click because of CenteredCanvas
         // How to get size of thGrid?
         double h = ((MainWindow)System.Windows.Application.Current.MainWindow).theGrid.Height;
         double w = ((MainWindow)System.Windows.Application.Current.MainWindow).theGrid.Width;

         clickPoint.Offset(-w / 2, -h / 2);

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

                  EditedCpVm.BuildGlobalShapeCollection(GetEditableCoBehaviourStateName(), ShowJoints, ShowSystems, ShowBackgrounds);

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

               EditedCpVm.BuildGlobalShapeCollection(GetEditableCoBehaviourStateName(), ShowJoints, ShowSystems, ShowBackgrounds);

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

               EditedCpVm.BuildGlobalShapeCollection(GetEditableCoBehaviourStateName(), ShowJoints, ShowSystems, ShowBackgrounds);

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
            svm.RotateShape(delta, fine);
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

      public int GetEditableCoBehaviourStateIndex()
      {
         if (EditedCpVm == null)
         {
            return 0;
         }

         if (!(EditedCpVm is CompoundObjectViewModel))
         {
            return 0;
         }

         CompoundObjectViewModel covm = EditedCpVm as CompoundObjectViewModel;

         return covm.Behaviour.DisplayedStateIndex;

      }

      public StateViewModel GetEditableCoBehaviourState()
      {
         if (EditedCpVm == null)
         {
            return null;
         }

         if (!(EditedCpVm is CompoundObjectViewModel))
         {
            return null;
         }

         CompoundObjectViewModel covm = EditedCpVm as CompoundObjectViewModel;

         return covm.Behaviour.States[GetEditableCoBehaviourStateIndex()];

      }

      public string GetEditableCoBehaviourStateName()
      {
         StateViewModel svm = GetEditableCoBehaviourState();

         if (svm != null)
         {
            return svm.StateName;
         }

         return "#Error should not reach here#";
      }

      public int GetEditableCoBehaviourIndexOf(IParentInterface child, string state)
      {
         FileCOViewModel fvm = GetFileViewModelOfChild(child);
         if ((fvm != null) && (fvm.Behaviour != null))
         {
            CompoundObjectViewModel covm = EditedCpVm as CompoundObjectViewModel;
            return fvm.Behaviour.States.IndexOf(covm.Behaviour.FindStateVM(state));
         }

         return 0;
      }

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

         SelectedChildObjects.Clear();
         SelectedChildObjectStateProperties.Clear();
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

            foreach (TreeViewViewModel tvvm in covm.ChildObjectsWithStates.Children)
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

                     foreach (ChildCOViewModel chcvm in chvm.StateProperties)
                     {
                        if (chcvm is ChildCOViewModel)
                        {
                           ChildCOViewModel cospvm = chcvm as ChildCOViewModel;

                           if (cospvm.IsSelected)
                           {
                              SelectedChildObjectStateProperties.Add(cospvm);
                           }
                        }
                     }
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

                                 foreach (ChildCOViewModel chcvm in chvm.StateProperties)
                                 {
                                    if (chcvm is ChildCOViewModel)
                                    {
                                       ChildCOViewModel cospvm = chcvm as ChildCOViewModel;

                                       if (cospvm.IsSelected)
                                       {
                                          SelectedChildObjectStateProperties.Add(cospvm);
                                       }
                                    }
                                 }
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

                        foreach (ChildCOViewModel chcvm in chvm.StateProperties)
                        {
                           if (chcvm is ChildCOViewModel)
                           {
                              ChildCOViewModel cospvm = chcvm as ChildCOViewModel;

                              if (cospvm.IsSelected)
                              {
                                 SelectedChildObjectStateProperties.Add(cospvm);
                              }
                           }
                        }
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

         foreach (FileCOViewModel coFile in FileCollectionViewModel)
         {
            coFile.IsSelected = false;
            coFile.DeselectAllChildren();
         }
      }

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

         ChildObjectStateProperties cosp = new ChildObjectStateProperties();
         cosp.File = fullFileName;

         TStateProperties<ChildObjectStateProperties> newStateProp = new TStateProperties<ChildObjectStateProperties>();
         newStateProp.Properties = cosp;

         CompoundObject newCP = CompoundObject.ReadFromFile(fullFileName);
         newStateProp.Properties.CompObj = newCP;

         ChildObject newChildObject = new ChildObject();
         newChildObject.StateProperties.Add(newStateProp);
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
         EditedCpVm.BuildGlobalShapeCollection(GetEditableCoBehaviourStateName(), ShowJoints, ShowSystems, ShowBackgrounds);

         EditedCpVm.OnPropertyChanged("");
         OnPropertyChanged("");

         Rect bib = EditedCpVm.BoundingBox;
         ((MainWindow)System.Windows.Application.Current.MainWindow).ShowThisRect(bib);

      }

      #endregion
   }
}
