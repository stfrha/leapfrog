using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;


/*
 * The CompoundObjectViewModel encompass the CompoundObject (incl its states, and behaviour) 
 * as well as all the ChildObjects of all states
 */


namespace LeapfrogEditor
{   // used to be: ConditionalSelectTreeViewViewModel
   public class CompoundObjectViewModel : TreeViewViewModel
   {
      #region Declarations

      private CompoundObject _modelObject;


      private CoBehaviourViewModel _behaviour;
      private int _selectedBehaviourIndex = 0;

      private StateShapeCollectionViewModel _shapes;
      private StateJointCollectionViewModel _joints;
      private StateSystemCollectionViewModel _systems;

      private StateChildCollectionViewModel _childObjectsWithStates;
//      private ObservableCollection<ObservableCollection<CompoundObjectViewModel>> _statesWithChildObjects = new ObservableCollection<ObservableCollection<CompoundObjectViewModel>>();

      private ObservableCollection<StateCollectionViewModelBase> _treeCollection = new ObservableCollection<StateCollectionViewModelBase>();
      #endregion

      #region Constructors

      public CompoundObjectViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm, 
         CompoundObject modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;

         Behaviour = new CoBehaviourViewModel(TreeParent, this, MainVm, ModelObject.Behaviour);

         SelectedBehaviourIndex = Behaviours.IndexOf(ModelObject.Behaviour.Type);

         ChildObjectsWithStates = new StateChildCollectionViewModel(treeParent, parentVm, MainVm);
      }

      #endregion

      #region Properties

/*
 * 
 * We need some new properties
 * 
 * Name -            this is the name of the ChildObject (if this is a child) or the 
 *                   file name (if this is a FileCOViewModel)
 * 
 * IsFileReference - true if this a ChidlObject that has a file reference. 
 *                   false, otherwise.
 *                   
 * FileName        - is the file reference of the ChildObject.stateProperties.file if
 *                   this is a ChildObject that has a file reference. If not, this is ""
 *                   
 */


      //public ChildObject ChildObjectOfParent
      //{
      //   get { return _childObjectOfParent; }
      //   set
      //   {
      //      _childObjectOfParent = value;
      //      OnPropertyChanged("");
      //   }
      //}

      public CompoundObject ModelObject
      {
         get { return _modelObject; }
         set
         {
            _modelObject = value;
            OnPropertyChanged("");
         }
      }

      public bool NeedsGroupIndex
      {
         get { return _modelObject.NeedsGroupIndex; }
         set
         {
            _modelObject.NeedsGroupIndex = value;
            OnPropertyChanged("NeedsGroupIndex");
         }
      }

      public CoBehaviourViewModel Behaviour
      {
         get { return _behaviour; }
         set
         {
            _behaviour = value;
            OnPropertyChanged("Behaviour");
         }
      }

      public virtual string Name
         // For a FileCOViewModel, this property supplies the file name and it can not be set.
         // The ChildCOViewModel overrides this Property to return the 
         // Child name  (and to be able to set it)
      {
         get
         {
            if (this is FileCOViewModel)
            {
               FileCOViewModel fcvm = this as FileCOViewModel;

               string fileName = System.IO.Path.GetFileName(fcvm.FileName);

               return fileName;
            }

            return "Error: could not resolve name";
         }
         set
         {
            // Name of top level Compound Object can not be set
         }
      }

      public StateShapeCollectionViewModel StateShapes
      {
         get { return _shapes; }
         set
         {
            _shapes = value;
            OnPropertyChanged("StateShapes");
         }
      }

      public StateJointCollectionViewModel StateJoints
      {
         get { return _joints; }
         set
         {
            _joints = value;
            OnPropertyChanged("StateJoints");
         }
      }

      public StateSystemCollectionViewModel StateSystems
      {
         get { return _systems; }
         set
         {
            _systems = value;
            OnPropertyChanged("StateSystems");
         }
      }
          

      public StateChildCollectionViewModel ChildObjectsWithStates
      {
         get { return _childObjectsWithStates; }
         set
         {
            _childObjectsWithStates = value;
            OnPropertyChanged("ChildObjectsWithStates");
         }
      }

      public List<string> Behaviours
      {
         get
         {
            return new List<string>()
            {
               "notApplicable",
               "scene",
               "leapfrog",
               "launchSite",
               "landingPad",
               "breakableObject",
               "explosiveObject",
               "steerableObject",
               "magneticMine",
               "pickupObject"
            };
         }
      }

      public int SelectedBehaviourIndex
      {
         get
         {
            return _selectedBehaviourIndex;
         }
         set
         {
            if (value == -1)
            {
               _selectedBehaviourIndex = 0;
            }
            else
            {
               _selectedBehaviourIndex = value;
            }
            OnPropertyChanged("SelectedBehaviourIndex");

            Behaviour.Type = Behaviours[SelectedBehaviourIndex];

            SetBehaviourPropertyInTreeView();

            // TODO: Except for updating the Type property 
            // which will expose a new BehaviourProperty,
            // we must also update the child of the Tree View
            // and this must be done here.
            // This ViewModel object is the Behaviour property of
            // the CompoundObjectViewModel. The COVM also has a TreeCollection
            // property which holds folders for all shapes, joints, systems, child
            // objects and Behaviour property. The Behaviour property should be the
            // set to either _steerableObjProperties or _breakableObjProperties
            // depending on the Type property. Both these types are derived of
            // BehaviourViewModelBase which is derived of TreeViewViewModel.
            // By letting BehaviourViewModelBase be derived from a new class that
            // is derived from StateCollectionViewModelBase, which basically is empty,
            // and derived from TreeViewViewModel it can be inserted into the TreeCollection
            // and thus will be displayed in the tree. 
         }
      }



      public ObservableCollection<StateCollectionViewModelBase> TreeCollection
      {
         get { return _treeCollection; }
         set { _treeCollection = value; }
      }

      public string DispState
      {
         get
         {
            if (MainVm != null)
            {
               StateViewModel svm = MainVm.GetEditableCoBehaviourState();

               if (svm != null)
               {
                  return svm.StateName;
               }
            }

            return "--nonExisting--";
         }
      }

      // Some functions can not differntiate between CompoundObjectViewMode
      // or ChildCOViewModel so these properties provide interface to the 
      // ChildCOVM if this instance is of that type
      public double PosX
      {
         get
         {
            if (this is ChildCOViewModel)
            {
               // PosX and Y are overridden in ChildCOViewModel so
               // this should really not be needed.
               ChildCOViewModel vm = this as ChildCOViewModel;

               return vm.PosX;
            }

            return 0;
         }
      }

      public double PosY
      {
         get
         {
            if (this is ChildCOViewModel)
            {
               ChildCOViewModel vm = this as ChildCOViewModel;

               return vm.PosY;
            }

            return 0;
         }
      }

      public Rect BoundingBox
      {
         get
         {
            if (StateShapes == null) return new Rect(0, 0, 100, 100);

            List<LfShapeViewModel> c = StateShapes.Shapes.OfType<LfShapeViewModel>().ToList();

            if ((c.Count == 0) && (ChildObjectsWithStates.Children.Count == 0))
            {
               return new Rect(0, 0, 100, 100);
            }

            BoundingBoxRect bbr = new BoundingBoxRect();

            if (StateShapes.Shapes.Count > 0)
            {
               foreach (object o in StateShapes.Shapes)
               {
                  if (o is LfShapeViewModel)
                  {
                     LfShapeViewModel shape = (LfShapeViewModel)o;

                     Rect cb = shape.BoundingBox;
                     cb.Offset(new Vector(shape.PosX, shape.PosY));
                     bbr.AddRect(cb);
                  }
               }
            }

            if (ChildObjectsWithStates.Children.Count > 0)
            {
               foreach (ChildObjectViewModel child in ChildObjectsWithStates.Children)
               {
                  foreach (ChildCOViewModel chvm in child.StateProperties)
                  {
                     if (chvm.State == MainVm.GetEditableCoBehaviourState().StateName)
                     {
                        Rect cb = chvm.BoundingBox;
                        cb.Offset(new Vector(chvm.PosX, chvm.PosY));
                        bbr.AddRect(cb);
                     }
                  }
               }
            }

            if (bbr.BoundingBox.IsEmpty)
            {
               return new Rect(0, 0, 100, 100);
            }

            return bbr.BoundingBox;
         }
      }

      #endregion

      #region Private Methods

      private StateShapeCollectionViewModel SetShapes(CompoundObject co, bool enabledChildren = true)
      {
         StateShapeCollectionViewModel shapes = new StateShapeCollectionViewModel(this, this, MainVm, enabledChildren);

         foreach (LfSpriteBox sb in co.SpriteBoxes)
         {
            LfSpriteBoxViewModel shapevm = new LfSpriteBoxViewModel(shapes, this, MainVm, sb, enabledChildren);
            shapes.Shapes.Add(shapevm);
         }

         foreach (LfSpritePolygon sp in co.SpritePolygons)
         {
            LfSpritePolygonViewModel shapevm = new LfSpritePolygonViewModel(shapes, this, MainVm, sp, enabledChildren);

            foreach (LfDragablePoint dragPoint in sp.Points)
            {
               LfDragablePointViewModel dragPointVm = new LfDragablePointViewModel(shapevm, this, MainVm, shapevm, dragPoint, enabledChildren);
               shapevm.PointVms.Add(dragPointVm);
            }

            shapes.Shapes.Add(shapevm);
         }

         foreach (LfStaticBox sb in co.StaticBoxes)
         {
            LfStaticBoxViewModel shapevm = new LfStaticBoxViewModel(shapes, this, MainVm, sb, enabledChildren);
            shapes.Shapes.Add(shapevm);
         }

         foreach (LfStaticCircle sb in co.StaticCircles)
         {
            LfStaticCircleViewModel shapevm = new LfStaticCircleViewModel(shapes, this, MainVm, sb, enabledChildren);
            shapes.Shapes.Add(shapevm);
         }

         foreach (LfStaticPolygon sp in co.StaticPolygons)
         {
            LfStaticPolygonViewModel shapevm = new LfStaticPolygonViewModel(shapes, this, MainVm, sp, enabledChildren);

            foreach (LfDragablePoint dragPoint in sp.Points)
            {
               LfDragablePointViewModel dragPointVm = new LfDragablePointViewModel(shapevm, this, MainVm, shapevm, dragPoint, enabledChildren);
               shapevm.PointVms.Add(dragPointVm);
            }

            shapes.Shapes.Add(shapevm);
         }

         foreach (LfDynamicBox db in co.DynamicBoxes)
         {
            LfDynamicBoxViewModel shapevm = new LfDynamicBoxViewModel(shapes, this, MainVm, db, enabledChildren);
            shapes.Shapes.Add(shapevm);
         }

         foreach (LfDynamicCircle db in co.DynamicCircles)
         {
            LfDynamicCircleViewModel shapevm = new LfDynamicCircleViewModel(shapes, this, MainVm, db, enabledChildren);
            shapes.Shapes.Add(shapevm);
         }

         foreach (LfDynamicPolygon dp in co.DynamicPolygons)
         {
            LfDynamicPolygonViewModel shapevm = new LfDynamicPolygonViewModel(shapes, this, MainVm, dp, enabledChildren);

            foreach (LfDragablePoint dragPoint in dp.Points)
            {
               LfDragablePointViewModel dragPointVm = new LfDragablePointViewModel(shapevm, this, MainVm, shapevm, dragPoint, enabledChildren);
               shapevm.PointVms.Add(dragPointVm);
            }

            shapes.Shapes.Add(shapevm);
         }

         foreach (LfStaticBoxedSpritePolygon bsp in co.StaticBoxedSpritePolygons)
         {
            LfStaticBoxedSpritePolygonViewModel shapevm = new LfStaticBoxedSpritePolygonViewModel(shapes, this, MainVm, bsp, enabledChildren);

            foreach (LfDragablePoint dragPoint in bsp.Points)
            {
               LfDragablePointViewModel dragPointVm = new LfDragablePointViewModel(shapevm, this, MainVm, shapevm, dragPoint, enabledChildren);
               shapevm.PointVms.Add(dragPointVm);
            }

            shapes.Shapes.Add(shapevm);
         }

         foreach (LfDynamicBoxedSpritePolygon bsp in co.DynamicBoxedSpritePolygons)
         {
            LfDynamicBoxedSpritePolygonViewModel shapevm = new LfDynamicBoxedSpritePolygonViewModel(shapes, this, MainVm, bsp, enabledChildren);

            foreach (LfDragablePoint dragPoint in bsp.Points)
            {
               LfDragablePointViewModel dragPointVm = new LfDragablePointViewModel(shapevm, this, MainVm, shapevm, dragPoint, enabledChildren);
               shapevm.PointVms.Add(dragPointVm);
            }

            shapes.Shapes.Add(shapevm);
         }

         return shapes;

      }

      private StateJointCollectionViewModel SetJoints(CompoundObject co, bool enabledChildren = true)
      {
         StateJointCollectionViewModel joints = new StateJointCollectionViewModel(this, this, MainVm, enabledChildren);

         foreach (WeldJoint wj in co.WeldJoints)
         {
            WeldJointViewModel wjvm = new WeldJointViewModel(joints, this, MainVm, wj, enabledChildren);
            joints.Joints.Add(wjvm);
         }

         foreach (RevoluteJoint rj in co.RevoluteJoints)
         {
            RevoluteJointViewModel rjvm = new RevoluteJointViewModel(joints, this, MainVm, rj, enabledChildren);
            joints.Joints.Add(rjvm);
         }

         foreach (PrismaticJoint pj in co.PrismaticJoints)
         {
            PrismaticJointViewModel pjvm = new PrismaticJointViewModel(joints, this, MainVm, pj, enabledChildren);
            joints.Joints.Add(pjvm);
         }

         foreach (Rope r in co.Ropes)
         {
            RopeViewModel rvm = new RopeViewModel(joints, this, MainVm, r, enabledChildren);
            joints.Joints.Add(rvm);
         }

         return joints;
      }

      private StateSystemCollectionViewModel SetSystems(CompoundObject co, bool enabledChildren = true)
      {
         StateSystemCollectionViewModel systems = new StateSystemCollectionViewModel(this, this, MainVm, enabledChildren);

         foreach (CoSystem s in co.Systems)
         {
            CoSystemViewModel svm = new CoSystemViewModel(systems, this, MainVm, s, enabledChildren);
            systems.Systems.Add(svm);
         }

         return systems;
      }


      private CompoundObjectViewModel FindCompoundObjectViewModelInChildrenObjects(ChildObjectStateProperties findMe)
      {
         foreach (ChildObjectViewModel childvm in ChildObjectsWithStates.Children)
         {
            foreach (ChildCOViewModel chospvm in childvm.StateProperties)
            {
               if (chospvm.ChildStateModelObject.Properties == findMe)
               {
                  return chospvm;
               }
            }
         }

         return null;
      }

      // All children CompoundObjectViewModels are created here. The constructor of the ChildObjectViewModel
      // creates a ChildObjectStatePropertiesViewModel which's constructor creates the CompoundObjectViewModel.
      private StateChildCollectionViewModel SetChildren(CompoundObject ModelObject, bool enabledChildren = true)
      {
         StateChildCollectionViewModel schcvm = new StateChildCollectionViewModel(this, this, MainVm, enabledChildren);

         foreach (ChildObject cho in ModelObject.ChildObjects)
         {
            ChildObjectViewModel chovm = new ChildObjectViewModel(schcvm, this, MainVm, cho, enabledChildren);

            schcvm.Children.Add(chovm);
         }

         return schcvm;
      }

      #endregion

      #region Public Methods

      public void SetBehaviourPropertyInTreeView()
      {
         // Remove the current element (if any) in the TreeCollection that is of BehaviourViewModelBase
         foreach (StateCollectionViewModelBase s in TreeCollection)
         {
            if (s is BehaviourViewModelBase)
            {
               TreeCollection.Remove(s);
               break;
            }
         }

         if ((Behaviour != null) && 
            ((Behaviour.Type == "breakableObject") || (Behaviour.Type == "scene")))
         {
            TreeCollection.Insert(0, Behaviour.BehaviourProperties);
         }
      }

      public void BuildTreeViewCollection()
      {
         _treeCollection.Clear();

         SetBehaviourPropertyInTreeView();

         _treeCollection.Add(StateShapes);
         _treeCollection.Add(StateJoints);
         _treeCollection.Add(StateSystems);
         _treeCollection.Add(ChildObjectsWithStates);
      }

      public void RemoveShape(LfShapeViewModel svm)
      {
         // Check if there are any joints connected to this svm, if so, removed them
         // We may remove joints so we need a for loop here:
         for (int i = StateJoints.Joints.Count - 1; i >= 0; i--)
         {
            // Below will take care of all joints since they
            // all inherit from WeldJoint
            if (StateJoints.Joints[i] is WeldJointViewModel)
            {
               WeldJointViewModel joint = (WeldJointViewModel)StateJoints.Joints[i];

               if ((joint.AName == svm.Name) || (joint.BName == svm.Name))
               {
                  // Remove the joint
                  ModelObject.RemoveJoint(joint.ModelObject);
                  StateJoints.Joints.RemoveAt(i);
               }
            }
         }

         // Check if there are any systems connected to this svm, if so, removed them
         // We may remove systems so we need a for loop here:
         for (int i = StateSystems.Systems.Count - 1; i >= 0; i--)
         {
            if (StateSystems.Systems[i] is CoSystemViewModel)
            {
               CoSystemViewModel system = (CoSystemViewModel)StateSystems.Systems[i];

               string bodyName = "";

               if (system.Properties is BodyOriginSystemViewModel)
               {
                  BodyOriginSystemViewModel prop = system.Properties as BodyOriginSystemViewModel;
                  bodyName = prop.BodyName;
               }
               else if (system.Properties is ObjectFactoryPropertiesViewModel)
               {
                  ObjectFactoryPropertiesViewModel prop = system.Properties as ObjectFactoryPropertiesViewModel;
                  bodyName = prop.Body;
               }
               else if (system.Properties is ReentryFlameEmitterPropertiesViewModel)
               {
                  ReentryFlameEmitterPropertiesViewModel prop = system.Properties as ReentryFlameEmitterPropertiesViewModel;
                  bodyName = prop.BodyName;
               }

               if (bodyName == svm.Name)
               {
                  // Remove the system
                  ModelObject.Systems.Remove(system.LocalModelObject);
                  StateSystems.Systems.RemoveAt(i);
               }
            }
         }

         // Remove the shape model
         ModelObject.RemoveShape(svm.ModelObject);

         // Remove the shape viewmodel from this
         StateShapes.Shapes.Remove(svm);

         // If there are no more shapes in the CO, remove the CO
         if (StateShapes.Shapes.Count == 0)
         {
            //ParentVm.StateChildObjects.Remove(this);
            //ParentVm.ModelObject.ChildObjectRefs(this.ChildObjectOfParent)
         }

         OnPropertyChanged("");
      }

      public void InvalidateJoints()
      {
         foreach (object o in StateJoints.Joints)
         {
            // Below will take care of all joints since they
            // all inherit from WeldJoint
            if (o is WeldJointViewModel)
            {
               WeldJointViewModel joint = (WeldJointViewModel)o;

               joint.OnPropertyChanged("");
            }
         }
      }

      public void InvalidateChildObjects()
      {
         foreach (ChildObjectViewModel chvm in _childObjectsWithStates.Children)
         {
            foreach (ChildCOViewModel spvm in chvm.StateProperties)
            {
               spvm.OnPropertyChanged("");
            }
         }
      }

      public LfShapeViewModel FindShape(string name, StateShapeCollectionViewModel shapes)
      {
         foreach (object o in shapes.Shapes)
         {
            if (o is LfShapeViewModel)
            {
               LfShapeViewModel shape = (LfShapeViewModel)o;

               if (shape.Name == name)
               {
                  return shape;
               }
            }
         }

         return null;
      }

      public void BuildViewModel(bool enabledChildren = true)
      // This method connects references in the COVM. At the creation of the COVM
      // the ModelObject, ModelObjectProperties, ParentVm and ChildObjectOfParent is defined.
      // Here we build the shape, joint and system collections and connect those objects 
      // with the necessary references. The ChildObjects are also added to the corresponding
      // state index in the StateChildObjects collection
      {
//         _statesWithChildObjects.Clear();
         _treeCollection.Clear();

         _shapes = SetShapes(ModelObject, enabledChildren);
         _joints = SetJoints(ModelObject, enabledChildren);
         _systems = SetSystems(ModelObject, enabledChildren);

         // Only now is the Joints property valid for this state
         foreach (object o in _joints.Joints)
         {
            if (o is WeldJointViewModel)
            {
               if (o is RopeViewModel)
               {
                  RopeViewModel joint = (RopeViewModel)o;

                  joint.ConnectToShapes(_shapes);
               }
               else
               {
                  WeldJointViewModel joint = (WeldJointViewModel)o;

                  joint.ConnectToShapes(_shapes);
               }
            }
         }

         // Only now is the Systems property valid for this state
         foreach (object o in _systems.Systems)
         {
            if (o is CoSystemViewModel)
            {
               CoSystemViewModel sys = o as CoSystemViewModel;

               if (sys.Properties is BodyOriginSystemViewModel)
               {
                  BodyOriginSystemViewModel sysProp = sys.Properties as BodyOriginSystemViewModel;
                  sysProp.ConnectToShapes(_shapes);
               }

               if (sys.Properties is ReentryFlameEmitterPropertiesViewModel)
               {
                  ReentryFlameEmitterPropertiesViewModel sysProp = sys.Properties as ReentryFlameEmitterPropertiesViewModel;
                  sysProp.ConnectToShapes(_shapes);
               }
            }
         }

         ChildObjectsWithStates = SetChildren(ModelObject, enabledChildren);

         BuildTreeViewCollection();
      }

      public void DeselectAllChildren()
      {
         if ((Behaviour != null) && (Behaviour.BehaviourProperties != null))
         {
            Behaviour.BehaviourProperties.IsSelected = false;

            if (Behaviour.BehaviourProperties is BreakableObjectPropertiesViewModel)
            {
               BreakableObjectPropertiesViewModel bovm = Behaviour.BehaviourProperties as BreakableObjectPropertiesViewModel;

               foreach (object o in bovm.SpawnObjects)
               {
                  if (o is SpawnObjectViewModel)
                  {
                     SpawnObjectViewModel sovm = o as SpawnObjectViewModel;

                     sovm.IsSelected = false;
                     sovm.SpawnChildObject[0].IsSelected = false;

                     foreach (object p in sovm.SpawnChildObject[0].StateProperties)
                     {
                        if (p is ChildCOViewModel)
                        {
                           ChildCOViewModel ccovm = p as ChildCOViewModel;

                           ccovm.IsSelected = false;
                        }
                     }
                  }
               }
            }
            else if (Behaviour.BehaviourProperties is ScenePropertiesViewModel)
            {
               // Deselect all the backgrounds
               ScenePropertiesViewModel spvm = Behaviour.BehaviourProperties as ScenePropertiesViewModel;

               foreach (ParallaxBackgroundViewModel pbvm in spvm.ParallaxBackgroundCollection.Backgrounds)
               {
                  pbvm.IsSelected = false;
               }
            }
         }

         if ((StateShapes != null) && (StateShapes.Shapes != null))
         {
            foreach (object o in StateShapes.Shapes)
            {
               if (o is LfPolygonViewModel)
               {
                  LfPolygonViewModel pvm = o as LfPolygonViewModel;

                  pvm.DeselectAllPoints();
                  pvm.IsSelected = false;
               }
               else if (o is LfShapeViewModel)
               {
                  LfShapeViewModel shape = o as LfShapeViewModel;

                  shape.IsSelected = false;
               }
            }
         }

         if (StateJoints != null)
         {
            foreach (object o in StateJoints.Joints)
            {
               if (o is WeldJointViewModel)
               {
                  WeldJointViewModel joint = (WeldJointViewModel)o;

                  joint.IsSelected = false;
               }
            }
         }

         if (StateSystems != null)
         {
            foreach (object o in StateSystems.Systems)
            {
               if (o is CoSystemViewModel)
               {
                  CoSystemViewModel system = (CoSystemViewModel)o;

                  system.IsSelected = false;

                  if ((system.Properties != null) && (system.Properties is TreeViewViewModel))
                  {
                     system.Properties.IsSelected = false;
                  }
               }
            }
         }

         if (ChildObjectsWithStates != null)
         {
            foreach (ChildObjectViewModel child in ChildObjectsWithStates.Children)
            {
               child.DeselectAllChildren();
               child.IsSelected = false;
            }
         }
      }


      public bool ChildHasFileReference(string fileName)
         // TODO: What was the purpose of this method?
      {
         foreach (ChildObjectViewModel chovm in ChildObjectsWithStates.Children)
         {
            foreach (ChildCOViewModel spvm in chovm.StateProperties)
            {
               if (spvm.File  == fileName)
               {
                  return true;
               }
               else
               {
                  if (spvm.ChildHasFileReference(fileName))
                  {
                     return true;
                  }
               }
            }
         }
         return false;
      }



      // This method returns with the point of the shape's coordinate system
      // in this CompoundObject's coordinate system. 
      // The position of the shape is added to the shape point. The shape point
      // can be either rotated or not.
      public Point ShapePointInCo(Point shapePoint, LfShapeViewModel shape)
      {
         if (shape == null) return new Point(0, 0);

         Point coPoint = shapePoint;

         coPoint.Offset(shape.PosX, shape.PosY);

         return coPoint;
      }

      // This method returns with the supplied point (expressed in this CompoundObject's
      // coordinate system) converted to the parent CompoundObjects coordinate system.
      // This is done by adding the Position of this CompoundObject.
      public virtual Point ParentCoPoint(Point coPoint)
      {
         // Since the top CO is always located at 0,0 there is no
         // coordinate transform at this level. Simply return the supplied point.
         return coPoint;
      }

      // This method returns with the point of the CO's coordinate system
      // in this Shape's coordinate system. 
      // The position of the shape is subtracted from the CO point. 
      public Point CoPointInShape(Point coPoint, LfShapeViewModel shape)
      {
         if (shape == null) return new Point(0, 0);

         Point shapePoint = coPoint;

         shapePoint.Offset(-shape.PosX, -shape.PosY);

         return shapePoint;
      }

      // This method returns with the supplied point in the scene's coordinate system
      // (Equal to the top level CompoundObject's coordinate system). 
      // The point is converted to the parent's coordinate system and the method is then
      // recursively called for the parent until the parent is null. In this case we are 
      // at the top level CompoundObject which is the scene. Then we returns the point
      public Point GetScenePointFromCoPoint(Point coPoint)
      {
         if (ParentVm != null)
         {
            Point parentPoint = ParentCoPoint(coPoint);
            return GetScenePointFromCoPoint(parentPoint);
         }
         else
         {
            return coPoint;
         }
      }

      public virtual Point GetCoPointFromScenePoint(Point scenePoint)
      {
         if (this is ChildCOViewModel)
         {
            ChildCOViewModel vm = this as ChildCOViewModel;

            Point parentPoint = vm.CoPointFromParent(scenePoint);
            return GetCoPointFromScenePoint(parentPoint);
         }
         else
         {
            return scenePoint;
         }
      }

      public void GenerateTriangles()
      {
         foreach (object o in StateShapes.Shapes)
         {
            if (o is LfPolygonViewModel)
            {
               LfPolygonViewModel pvm = (LfPolygonViewModel)o;

               pvm.GenerateTriangles();
            }
         }

         foreach (ChildObjectViewModel covm in ChildObjectsWithStates.Children)
         {
            foreach (ChildCOViewModel propvm in covm.StateProperties)
            {
               propvm.GenerateTriangles();
            }
         }
      }

      public LfShapeViewModel FindBodyObject(string bodyName)
      {
         foreach (object o in StateShapes.Shapes)
         {
            if (o is LfShapeViewModel)
            {
               LfShapeViewModel lsvm = o as LfShapeViewModel;

               if (lsvm.Name == bodyName)
               {
                  return lsvm;
               }
            }
         }

         return null;
      }

      public void UpdateAnglePropertyOfSystems()
      {
         foreach (object o in StateSystems.Systems)
         {
            if (o is CoSystemViewModel)
            {
               CoSystemViewModel sysVm = o as CoSystemViewModel;

               if (sysVm.Properties != null)
               {
                  sysVm.Properties.OnPropertyChanged("");
               }
            }
         }
      }

      #endregion

   }
}
