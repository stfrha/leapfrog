using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Media;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
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
      objectFactory,
      rope
   }

   public enum MouseEventObjectType
   {
      none,                         // object is null
      shape,                        // object is the shape
      dragablePolygonBorder,        // border that a point can be added to,
                                    // object is the point (before or after?) the line
      dragableBorder,               // object is the point (before or after?) the line
      dragablePoint,                // object is the point
      compoundObjectBoundaryBox,    // object is the CompoundObject
      jointAnchorA,                 // object is the joint
      jointAnchorB,                 // object is the joint
      prismJointUpperLimit,         // object is the prismatic joint
      prismJointLowerLimit          // object is the prismatic joint
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

      private bool _showJoints;
      private bool _showTriangles;

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
         }
      }

      public bool ShowTriangles
      {
         get { return _showTriangles; }
         set
         {
            _showTriangles = value;
            OnPropertyChanged("ShowTriangles");
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
            OpenFileToEdit(System.IO.Path.GetFileName(ofd.FileName));
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

            OpenFileToEdit(covm.File);
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

            EditedCpVm.GenerateTriangles();

            EditedCpVm.ModelObject.WriteToFile(fvm.FullPathFileName);

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
               EditedCpVm.GenerateTriangles();
               fvm.FileName = sfd.FileName;
               EditedCpVm.ModelObject.WriteToFile(fvm.FullPathFileName);
            }


            EditedCpVm.GenerateTriangles();

            EditedCpVm.ModelObject.WriteToFile(fvm.FullPathFileName);
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

      

      void DisplayStateExecute(Object parameter)
      {
         if (parameter is StateViewModel)
         {
            StateViewModel svm = parameter as StateViewModel;

            svm.SetOnDisplay();

            EditedCpVm.OnPropertyChanged("");
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
                  polyVm.ParentVm.StateShapes.Shapes.Remove(polyVm);
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
               covm.StateJoints.Joints.Remove(jvm);

            }
            SelectedJoints.Clear();

         }
         else if (SelectedShapes.Count > 0)
         {
            foreach (LfShapeViewModel svm in SelectedShapes)
            {
               CompoundObjectViewModel covm = svm.ParentVm;

               covm.ModelObject.RemoveShape(svm.ModelObject);
               covm.RemoveShape(svm);

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
            covm.StateJoints.Joints.Remove(jvm);
         }

         if (parameter is LfShapeViewModel)
         {
            LfShapeViewModel svm = parameter as LfShapeViewModel;
            CompoundObjectViewModel covm = svm.ParentVm;

            covm.ModelObject.RemoveShape(svm.ModelObject);
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
      }

      bool CanDeleteThisObjectExecute(Object parameter)
      {
         if (parameter == null)
         {
            return false;
         }

         if ((EditedCpVm != null) && ((parameter is LfShapeViewModel) || 
            (parameter is WeldJointViewModel) || (parameter is ChildCOViewModel) ||
            (parameter is ChildObjectViewModel)))
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
         if (parameter is StateChildCollectionViewModel)
         {
            StateChildCollectionViewModel scvm = parameter as StateChildCollectionViewModel;

            ChildObject ch = new ChildObject();
            ChildObjectViewModel chvm = new ChildObjectViewModel(scvm, EditedCpVm, this, ch, true);

            EditedCpVm.ModelObject.ChildObjects.Add(ch);
            scvm.Children.Add(chvm);
         }

         if (parameter is ChildObjectViewModel)
         {
            ChildObjectViewModel chvm = parameter as ChildObjectViewModel;

            TStateProperties<ChildObjectStateProperties> sp = new TStateProperties<ChildObjectStateProperties>();
            chvm.ModelObject.StateProperties.Add(sp);

            sp.Properties = new ChildObjectStateProperties();
            sp.Properties.CompObj = new CompoundObject();

            ChildCOViewModel covm = new ChildCOViewModel(chvm, EditedCpVm, this, sp);
            covm.BuildViewModel(true);

            chvm.StateProperties.Add(covm);
         }
      }

      bool CanAddChildObjectExecute(Object parameter)
      {
         if (parameter == null)
         {
            return false;
         }

         if ((EditedCpVm != null) && ((parameter is StateChildCollectionViewModel) || (parameter is ChildObjectViewModel)))
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

               case MouseEventObjectType.dragableBorder:
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

               return true;

            case MouseEventObjectType.dragablePoint:

               // Mouse move on rectangle of DragablePoint
               LfDragablePointViewModel dpvm = (LfDragablePointViewModel)sender;

               // Before moving all vertices, we rotate the vector to match the shape rotation.
               Point vectPoint = new Point(dragVector.X, dragVector.Y);
               Point rotPoint = dpvm.Parent.LocalPointFromRotated(vectPoint);
               Vector rotatedDragVector = new Vector(rotPoint.X, rotPoint.Y);
               
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
               rotPoint = wjvm.AShapeObject.LocalPointFromRotated(vectPoint);
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
               rotPoint = wjvm.BShapeObject.LocalPointFromRotated(vectPoint);
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
               rotPoint = pjvm.AShapeObject.LocalPointFromRotated(vectPoint);
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
               rotPoint = pjvm.AShapeObject.LocalPointFromRotated(vectPoint);
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

                  // If the clicked shape is children of the edited object, it should
                  // be selected (assessing the ctrl key in the process for multiple 
                  // selection)
                  // If it is not part of the Editable object, the parent should be selected
                  // also assessing ctrl-key

                  if (shvm.ParentVm == EditedCpVm)
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

               case MouseEventObjectType.jointAnchorA:
               case MouseEventObjectType.jointAnchorB:

                  WeldJointViewModel jvm = (WeldJointViewModel)sender;

                  if (jvm.ParentVm == EditedCpVm)
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

                     Point rotShapePoint = dpvm.Parent.LocalPointFromRotated(unrotatedShapePoint);

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
               (_LeftClickState == LeftClickState.spriteBox))
            {
               // The first point of this polygon will be the PosX and PosY of the 
               // new shape, and thus, the first polygon vertex should be at 0,0.
               Point parentOrigo = new Point(0, 0);
               Point localClickPoint = new Point();
               localClickPoint = (Point)(clickPoint - parentOrigo);

               LfShape newShape = null;
               LfShapeViewModel newShapeVm = null;

               if (_LeftClickState == LeftClickState.staticBox)
               {
                  newShape = new LfStaticBox();
                  newShapeVm = new LfStaticBoxViewModel(EditedCpVm.StateShapes, EditedCpVm, this, (LfStaticBox)newShape);
                  EditedCpVm.ModelObject.StaticBoxes.Add((LfStaticBox)newShape);
               }
               else if (_LeftClickState == LeftClickState.dynamicBox)
               {
                  newShape = new LfDynamicBox();
                  newShapeVm = new LfDynamicBoxViewModel(EditedCpVm.StateShapes, EditedCpVm, this, (LfDynamicBox)newShape);
                  EditedCpVm.ModelObject.DynamicBoxes.Add((LfDynamicBox)newShape);
               }
               else if (_LeftClickState == LeftClickState.staticCircle)
               {
                  newShape = new LfStaticCircle();
                  newShapeVm = new LfStaticCircleViewModel(EditedCpVm.StateShapes, EditedCpVm, this, (LfStaticCircle)newShape);
                  EditedCpVm.ModelObject.StaticCircles.Add((LfStaticCircle)newShape);
               }
               else if (_LeftClickState == LeftClickState.dynamicCircle)
               {
                  newShape = new LfDynamicCircle();
                  newShapeVm = new LfDynamicCircleViewModel(EditedCpVm.StateShapes, EditedCpVm, this, (LfDynamicCircle)newShape);
                  EditedCpVm.ModelObject.DynamicCircles.Add((LfDynamicCircle)newShape);
               }
               else if (_LeftClickState == LeftClickState.spriteBox)
               {
                  newShape = new LfSpriteBox();
                  newShapeVm = new LfSpriteBoxViewModel(EditedCpVm.StateShapes, EditedCpVm, this, (LfSpriteBox)newShape);
                  EditedCpVm.ModelObject.SpriteBoxes.Add((LfSpriteBox)newShape);
               }

               if ((newShape != null) && (newShapeVm != null))
               {
                  EditedCpVm.StateShapes.Shapes.Add(newShapeVm);

                  SelectedShapes.Add(newShapeVm);

                  newShapeVm.PosX = localClickPoint.X;
                  newShapeVm.PosY = localClickPoint.Y;

                  newShapeVm.IsSelected = true;

                  foreach (LfDragablePointViewModel selpoint in _selectedPoints)
                  {
                     selpoint.IsSelected = false;
                  }
                  _selectedPoints.Clear();
               }

               _LeftClickState = LeftClickState.none;

            }
            else if ((_LeftClickState == LeftClickState.staticPolygon) ||
               (_LeftClickState == LeftClickState.dynamicPolygon) ||
               (_LeftClickState == LeftClickState.spritePolygon))
            {
               // The first point of this polygon will be the PosX and PosY of the 
               // new shape, and thus, the first polygon vertex should be at 0,0.
               Point parentOrigo = new Point(0, 0);
               Point localClickPoint = new Point();
               localClickPoint = (Point)(clickPoint - parentOrigo);

               LfScalableTexturePolygon newPolygon;
               LfScalableTexturePolygonViewModel newPolygonVm;

               if (_LeftClickState == LeftClickState.staticPolygon)
               {
                  newPolygon = new LfStaticPolygon();
                  newPolygonVm = new LfStaticPolygonViewModel(EditedCpVm.StateShapes, EditedCpVm, this, (LfStaticPolygon)newPolygon);
                  EditedCpVm.ModelObject.StaticPolygons.Add((LfStaticPolygon)newPolygon);
               }
               else if (_LeftClickState == LeftClickState.dynamicPolygon)
               {
                  newPolygon = new LfDynamicPolygon();
                  newPolygonVm = new LfDynamicPolygonViewModel(EditedCpVm.StateShapes, EditedCpVm, this, (LfDynamicPolygon)newPolygon);
                  EditedCpVm.ModelObject.DynamicPolygons.Add((LfDynamicPolygon)newPolygon);
               }
               else
               {
                  newPolygon = new LfSpritePolygon();
                  newPolygonVm = new LfSpritePolygonViewModel(EditedCpVm.StateShapes, EditedCpVm, this, (LfSpritePolygon)newPolygon);
                  EditedCpVm.ModelObject.SpritePolygons.Add((LfSpritePolygon)newPolygon);
               }

               newPolygon.PosX = localClickPoint.X;
               newPolygon.PosY = localClickPoint.Y;

               EditedCpVm.StateShapes.Shapes.Add(newPolygonVm);

               SelectedShapes.Add(newPolygonVm);
               newPolygonVm.IsSelected = true;

               LfDragablePointViewModel newPoint = newPolygonVm.InsertPoint(new Point(0, 0), null);

               foreach (LfDragablePointViewModel selpoint in _selectedPoints)
               {
                  selpoint.IsSelected = false;
               }
               _selectedPoints.Clear();

               _selectedPoints.Add(newPoint);
               newPoint.IsSelected = true;

               _LeftClickState = LeftClickState.addPoint;

            }
            else if ((_LeftClickState == LeftClickState.staticBoxedSpritePolygon) ||
               (_LeftClickState == LeftClickState.dynamicBoxedSpritePolygon))
            {
               // The first point of this polygon will be the PosX and PosY of the 
               // new shape, and thus, the first polygon vertex should be at 0,0.
               Point parentOrigo = new Point(0, 0);
               Point localClickPoint = new Point();
               localClickPoint = (Point)(clickPoint - parentOrigo);

               LfStaticBoxedSpritePolygon newPolygon = null;
               LfStaticBoxedSpritePolygonViewModel newPolygonVm = null;

               if (_LeftClickState == LeftClickState.staticBoxedSpritePolygon)
               {
                  newPolygon = new LfStaticBoxedSpritePolygon();
                  newPolygonVm = new LfStaticBoxedSpritePolygonViewModel(EditedCpVm.StateShapes, EditedCpVm, this, (LfStaticBoxedSpritePolygon)newPolygon);
                  EditedCpVm.ModelObject.StaticBoxedSpritePolygons.Add((LfStaticBoxedSpritePolygon)newPolygon);
               }
               else
               {
                  newPolygon = new LfDynamicBoxedSpritePolygon();
                  newPolygonVm = new LfDynamicBoxedSpritePolygonViewModel(EditedCpVm.StateShapes, EditedCpVm, this, (LfDynamicBoxedSpritePolygon)newPolygon);
                  EditedCpVm.ModelObject.DynamicBoxedSpritePolygons.Add((LfDynamicBoxedSpritePolygon)newPolygon);
               }

               newPolygon.PosX = localClickPoint.X;
               newPolygon.PosY = localClickPoint.Y;

               EditedCpVm.StateShapes.Shapes.Add(newPolygonVm);

               SelectedShapes.Add(newPolygonVm);
               newPolygonVm.IsSelected = true;

               LfDragablePointViewModel newPoint = newPolygonVm.InsertPoint(new Point(0, 0), null);

               foreach (LfDragablePointViewModel selpoint in _selectedPoints)
               {
                  selpoint.IsSelected = false;
               }
               _selectedPoints.Clear();

               _selectedPoints.Add(newPoint);
               newPoint.IsSelected = true;

               _LeftClickState = LeftClickState.addPoint;

            }
            else if ((_LeftClickState == LeftClickState.weldJoint) ||
               (_LeftClickState == LeftClickState.revoluteJoint) ||
               (_LeftClickState == LeftClickState.prismaticJoint))
            {
               WeldJoint wj = null;

               if (_LeftClickState == LeftClickState.weldJoint)
               {
                  wj = new WeldJoint();
               }
               else if (_LeftClickState == LeftClickState.revoluteJoint)
               {
                  wj = new RevoluteJoint();
               }
               else
               {
                  wj = new PrismaticJoint();
               }

               wj.AName = SelectedShapes[0].Name;
               wj.BName = SelectedShapes[1].Name;

               if (wj.AName == wj.BName)
               {
                  MessageBox.Show("The selected shapes has the same name and thus a unambigous joint can not be created. Rename at least one of the shapes.", "Error Creating Joint", MessageBoxButton.OK, MessageBoxImage.Error);

                  _LeftClickState = LeftClickState.none;
                  return true;
               }

               WeldJointViewModel wjvm = null;

               if (_LeftClickState == LeftClickState.weldJoint)
               {
                  wjvm = new WeldJointViewModel(EditedCpVm.StateJoints, EditedCpVm, this, wj);
               }
               else if (_LeftClickState == LeftClickState.revoluteJoint)
               {
                  wjvm = new RevoluteJointViewModel(EditedCpVm.StateJoints, EditedCpVm, this, (RevoluteJoint)wj);
               }
               else
               {
                  wjvm = new PrismaticJointViewModel(EditedCpVm.StateJoints, EditedCpVm, this, (PrismaticJoint)wj);
               }

               wjvm.ConnectToShapes(EditedCpVm.StateShapes);

               Point parentObjectOrigo = new Point(0, 0);

               // Shape A point
               Point shapeAOrigo = new Point(wjvm.AShapeObject.PosX, wjvm.AShapeObject.PosY);
               shapeAOrigo.Offset(parentObjectOrigo.X, parentObjectOrigo.Y);
               Point localAClickPoint = new Point();
               localAClickPoint = (Point)(clickPoint - shapeAOrigo);

               // Rotate point to shape rotation
//               Point rotatedAClickPoint = wjvm.AShapeObject.RotatedPointFromLocal(localAClickPoint);
               Point rotatedAClickPoint = wjvm.AShapeObject.LocalPointFromRotated(localAClickPoint);

               wjvm.AAnchorX = rotatedAClickPoint.X;
               wjvm.AAnchorY = rotatedAClickPoint.Y;

               // Shape B point
               Point shapeBOrigo = new Point(wjvm.BShapeObject.PosX, wjvm.BShapeObject.PosY);
               shapeBOrigo.Offset(parentObjectOrigo.X, parentObjectOrigo.Y);
               Point localBClickPoint = new Point();
               localBClickPoint = (Point)(clickPoint - shapeBOrigo);

               // Rotate point to shape rotation
//               Point rotatedBClickPoint = wjvm.BShapeObject.RotatedPointFromLocal(localBClickPoint);
               Point rotatedBClickPoint = wjvm.BShapeObject.LocalPointFromRotated(localBClickPoint);

               wjvm.BAnchorX = rotatedBClickPoint.X;
               wjvm.BAnchorY = rotatedBClickPoint.Y;

               EditedCpVm.StateJoints.Joints.Add(wjvm);

               if (_LeftClickState == LeftClickState.weldJoint)
               {
                  EditedCpVm.ModelObject.WeldJoints.Add(wj);
               }
               else if (_LeftClickState == LeftClickState.revoluteJoint)
               {
                  EditedCpVm.ModelObject.RevoluteJoints.Add((RevoluteJoint)wj);
               }
               else
               {
                  EditedCpVm.ModelObject.PrismaticJoints.Add((PrismaticJoint)wj);
               }

               _LeftClickState = LeftClickState.none;

            }
            else if (_LeftClickState == LeftClickState.rope)
            {
               Rope rp = new Rope();

               rp.AName = SelectedShapes[0].Name;

               if (SelectedShapes.Count == 2)
               {
                  rp.BName = SelectedShapes[1].Name;

                  if (rp.AName == rp.BName)
                  {
                     MessageBox.Show("The selected shapes has the same name and thus a unambigous joint can not be created. Rename at least one of the shapes.", "Error Creating Joint", MessageBoxButton.OK, MessageBoxImage.Error);

                     _LeftClickState = LeftClickState.none;
                     return true;
                  }
               }

               // If there is no shape B, the rp.BName should be default "notDef"

               RopeViewModel rpvm = new RopeViewModel(EditedCpVm.StateJoints, EditedCpVm, this, rp);

               rpvm.ConnectToShapes(EditedCpVm.StateShapes);

               Point parentObjectOrigo = new Point(0, 0);

               // Shape A point
               Point shapeAOrigo = new Point(rpvm.AShapeObject.PosX, rpvm.AShapeObject.PosY);
               shapeAOrigo.Offset(parentObjectOrigo.X, parentObjectOrigo.Y);
               Point localAClickPoint = new Point();
               localAClickPoint = (Point)(clickPoint - shapeAOrigo);

               // Rotate point to shape rotation
               Point rotatedAClickPoint = rpvm.AShapeObject.LocalPointFromRotated(localAClickPoint);

               rpvm.AAnchorX = rotatedAClickPoint.X;
               rpvm.AAnchorY = rotatedAClickPoint.Y;

               if (SelectedShapes.Count == 2)
               {
                  // Shape B point
                  Point shapeBOrigo = new Point(rpvm.BShapeObject.PosX, rpvm.BShapeObject.PosY);
                  shapeBOrigo.Offset(parentObjectOrigo.X, parentObjectOrigo.Y);
                  Point localBClickPoint = new Point();
                  localBClickPoint = (Point)(clickPoint - shapeBOrigo);

                  // Rotate point to shape rotation
                  Point rotatedBClickPoint = rpvm.BShapeObject.LocalPointFromRotated(localBClickPoint);

                  rpvm.BAnchorX = rotatedBClickPoint.X;
                  rpvm.BAnchorY = rotatedBClickPoint.Y;
               }

               EditedCpVm.StateJoints.Joints.Add(rpvm);
               EditedCpVm.ModelObject.Ropes.Add(rp);

               _LeftClickState = LeftClickState.none;

            }
/*
            else if (_LeftClickState == LeftClickState.objectFactory)
            {
               // The first point of this polygon will be the PosX and PosY of the 
               // new shape, and thus, the first polygon vertex should be at 0,0.
               Point parentOrigo = new Point(SelectedChildObjects.PosX, SelectedChildObjects.PosY);
               Point localClickPoint = new Point();
               localClickPoint = (Point)(clickPoint - parentOrigo);

               ObjectFactoryRef afr = new ObjectFactoryRef();

               TStateProperties<ObjectFactoryProperties> sp = new TStateProperties<ObjectFactoryProperties>();

               afr.StateProperties.Add(sp);

               ObjectFactoryProperties afp = new ObjectFactoryProperties();
               afp.PosX = localClickPoint.X;
               afp.PosY = localClickPoint.Y;

               sp.Properties = afp;

               ObjectFactoryViewModel afvm = new ObjectFactoryViewModel(this, SelectedChildObjects, afr);

               SelectedChildObjects.ModelObject.ObjectFactories.Add(afr);

               SelectedChildObjects.Shapes.Add(afvm);

               SelectedShapes.Add(afvm);
               afvm.IsSelected = true;

               _LeftClickState = LeftClickState.none;

            }
            */
            else if (_LeftClickState == LeftClickState.addPoint)
            {
               // When adding points to new polygon we require that the
               // shape is the only one selected
               if ((SelectedShapes.Count == 1) && (SelectedShapes[0] is LfPolygonViewModel))
               {
                  LfPolygonViewModel newPolygon = (LfPolygonViewModel)SelectedShapes[0];
                  Point parentObjectOrigo = new Point(newPolygon.ParentVm.PosX, newPolygon.ParentVm.PosY);
                  Point shapeOrigo = new Point(newPolygon.PosX, newPolygon.PosY);
                  shapeOrigo.Offset(parentObjectOrigo.X, parentObjectOrigo.Y);
                  Point localClickPoint = new Point();
                  localClickPoint = (Point)(clickPoint - shapeOrigo);

                  LfDragablePointViewModel newPoint = newPolygon.AddPoint(localClickPoint);
                  foreach (LfDragablePointViewModel selpoint in _selectedPoints)
                  {
                     selpoint.IsSelected = false;
                  }
                  _selectedPoints.Clear();

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
            //if (_selectedPoints.Count > 0)
            //{
            //   foreach (LfDragablePointViewModel dp in _selectedPoints)
            //   {
            //      LfPolygonViewModel polyVm = dp.ParentVm;

            //      // Is this the last point to be removed? If so, remove the shape
            //      // first so there is no problem with updating something with zero
            //      // points.
            //      if (polyVm.PointVms.Count == 1)
            //      {
            //         // Polygon has no more points, delete the polygon Shape

            //         polyVm.ParentVm.ModelObject.RemoveShape(polyVm.ModelObject);
            //         polyVm.ParentVm.Shapes.Remove(polyVm);
            //      }

            //      // Before we remove the point
            //      polyVm.RemovePoint(dp);

            //   }
            //   _selectedPoints.Clear();
            //}
         }

         return false;
      }

      public void RotateSelectedShape(int delta, bool fine)
      {
         foreach (LfShapeViewModel svm in SelectedShapes)
         {
            svm.RotateShape(delta, fine);
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

         return EditedCpVm.Behaviour.DisplayedStateIndex;

      }

      public StateViewModel GetEditableCoBehaviourState()
      {
         if (EditedCpVm == null)
         {
            return null;
         }

         return EditedCpVm.Behaviour.States[GetEditableCoBehaviourStateIndex()];

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

      public int GetEditableCoBehaviourIndexOf(string state)
      {
         return EditedCpVm.Behaviour.States.IndexOf(EditedCpVm.Behaviour.FindStateVM(state));
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

         foreach (TreeViewViewModel tvvm in EditedCpVm.ChildObjectsWithStates.Children)
         {
            if (tvvm is ChildObjectViewModel)
            {
               ChildObjectViewModel covm = tvvm as ChildObjectViewModel;

               if (covm.ParentVm == EditedCpVm)
               {
                  // This is the child object of the object being edited, 
                  if (covm.IsSelected)
                  {
                     SelectedChildObjects.Add(covm);
                  }

                  foreach (ChildCOViewModel chvm in covm.StateProperties)
                  {
                     if (chvm is ChildCOViewModel)
                     {
                        ChildCOViewModel cospvm = chvm as ChildCOViewModel;

                        if (cospvm.IsSelected)
                        {
                           SelectedChildObjectStateProperties.Add(cospvm);
                        }
                     }
                  }
               }
            }
         }

         foreach (object o in EditedCpVm.StateShapes.Shapes)
         {
            if (o is LfShapeViewModel)
            {
               LfShapeViewModel shape = o as LfShapeViewModel;

               if (shape.ParentVm == EditedCpVm)
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

         foreach (object o in EditedCpVm.StateJoints.Joints)
         {
            if (o is WeldJointViewModel)
            {
               WeldJointViewModel joint = o as WeldJointViewModel;

               if (joint.ParentVm == EditedCpVm)
               {
                  if (joint.IsSelected)
                  {
                     // This is the shape of the object being edited, 
                     SelectedJoints.Add(joint);
                  }
               }
            }
         }

         foreach (object o in EditedCpVm.StateSystems.Systems)
         {
            if (o is CoSystemViewModel)
            {
               CoSystemViewModel system = o as CoSystemViewModel;

               if (system.ParentVm == EditedCpVm)
               {
                  if (system.IsSelected)
                  {
                     // This is the shape of the object being edited, 
                     SelectedSystems.Add(system);
                  }
               }
            }
         }


         //if (tvvm is SpawnObjectViewModel)
         //{
         //   EditableSpawnObject = tvvm as SpawnObjectViewModel;
         //}
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

      private void OpenFileToEdit(string fileName)
      {
         // Lets first look to see if this file is not already openend
         FileCOViewModel fcovm = FindOpenedFile(fileName);

         if (fcovm != null)
         {
            // If file is already opened, we simply edit it
            EditedCpVm = fcovm;
            EditedCpVm.OnPropertyChanged("");
            OnPropertyChanged("");

            Rect bib = EditedCpVm.BoundingBox;
            ((MainWindow)System.Windows.Application.Current.MainWindow).ShowThisRect(bib);

            return;
         }

         FileCOViewModel newCpVm = OpenFile(fileName);

         FileCollectionViewModel.Add(newCpVm);

         EditedCpVm = newCpVm;
         EditedCpVm.BuildViewModel();
         EditedCpVm.OnPropertyChanged("");
         OnPropertyChanged("");

         Rect bb = EditedCpVm.BoundingBox;
         ((MainWindow)System.Windows.Application.Current.MainWindow).ShowThisRect(bb);


         //((MainWindow)System.Windows.Application.Current.MainWindow).TheTreeView.
      }

      #endregion
   }
}
