﻿using System;
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

      private CompositeCollection _globalShapeCollection;

      private CoBehaviourViewModel _behaviour;
      private int _selectedBehaviourIndex = 0;

      private ShapeCollectionViewModel _shapes;
      private JointCollectionViewModel _joints;
      private SystemCollectionViewModel _systems;

      private ChildCollectionViewModel _childObjects;

      // There are no corresponding model class for imported objects
      private ImportedObjectCollectionViewModel _importedObjects;

      private ObservableCollection<CollectionViewModelBase> _treeCollection = new ObservableCollection<CollectionViewModelBase>();
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

         ChildObjectCollection = new ChildCollectionViewModel(treeParent, parentVm, MainVm);

         ImportedObjects = new ImportedObjectCollectionViewModel(treeParent, parentVm, mainVm);

         _globalShapeCollection = new CompositeCollection()
         {
            new CollectionContainer { Collection = new ObservableCollection<LfSpriteBoxViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfSpritePolygonViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfStaticBoxViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfStaticCircleViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfStaticPolygonViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfStaticBoxedSpritePolygonViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfDynamicBoxViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfDynamicCircleViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfDynamicPolygonViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfDynamicBoxedSpritePolygonViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<WeldJointViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<RevoluteJointViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<PrismaticJointViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<RopeViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<CoSystemViewModel>() },
         };

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

      public CompoundObject ModelObject
      {
         get { return _modelObject; }
         set
         {
            _modelObject = value;
            OnPropertyChanged("");
         }
      }

      public CompositeCollection GlobalShapeCollection
      // The GlobalShapeCollection is the collection of all shapes in one single
      // heap, used to be able to handle z-level in the view. If this is a imported
      // file, we want to display all shapes, even those of the top-level scene 
      // and all currently imported files. Thus, in this case, we expose the top-level
      // parent's GLobalShapeCollection instead of the local one.
      {
         get
         {
            if (AmIAnImportedFile())
            {
               return TopParentOfImporedFiles().GlobalShapeCollection;
            }

            return _globalShapeCollection;
         }
         set { _globalShapeCollection = value; }
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

      virtual public string Name
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

      public ShapeCollectionViewModel ShapeCollection
      {
         get { return _shapes; }
         set
         {
            _shapes = value;
            OnPropertyChanged("ShapeCollection");
         }
      }

      public JointCollectionViewModel JointCollection
      {
         get { return _joints; }
         set
         {
            _joints = value;
            OnPropertyChanged("JointCollection");
         }
      }

      public SystemCollectionViewModel SystemCollection
      {
         get { return _systems; }
         set
         {
            _systems = value;
            OnPropertyChanged("SystemCollection");
         }
      }
          

      public ChildCollectionViewModel ChildObjectCollection
      {
         get { return _childObjects; }
         set
         {
            _childObjects = value;
            OnPropertyChanged("ChildObjectCollection");
         }
      }

      public ImportedObjectCollectionViewModel ImportedObjects
      {
         get { return _importedObjects; }
         set
         {
            _importedObjects = value;
            OnPropertyChanged("ImportedObjects");
         }
      }

      public bool IsImportedFile
      {
         get { return AmIAnImportedFile(); }
      }

      public List<string> Behaviours
      {
         get
         {
            return new List<string>()
            {
               "notApplicable",
               "scene",
               "parallaxBackground",
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



      public ObservableCollection<CollectionViewModelBase> TreeCollection
      {
         get { return _treeCollection; }
         set { _treeCollection = value; }
      }

      //public string DispState
      //{
      //   get
      //   {
      //      if (MainVm != null)
      //      {
      //         StateViewModel svm = MainVm.GetEditableCoBehaviourState();

      //         if (svm != null)
      //         {
      //            return svm.StateName;
      //         }
      //      }

      //      return "--nonExisting--";
      //   }
      //}

      // Some functions can not differntiate between CompoundObjectViewMode
      // or ChildCOViewModel so these properties provide interface to the 
      // ChildCOVM if this instance is of that type
      public double PosX
      {
         get
         {
            return 0;
         }
      }

      public double AbsPosX
      {
         get
         {
            if (this != MainVm.EditedCpVm)
            {
               if ((TreeParent != null) && (TreeParent is ChildObjectViewModel))
               {
                  ChildObjectViewModel covm = TreeParent as ChildObjectViewModel;

                  return covm.AbsPosX;
               }
            }


            //if (this != MainVm.EditedCpVm)
            //{
            //   double parentPos = 0;

            //   if (ParentVm != null)
            //   {
            //      parentPos = ParentVm.AbsPosX;
            //   }

            //   return parentPos + PosX;
            //}

            return 0;
         }
      }

      public double PosY
      {
         get
         {
            return 0;
         }
      }

      public double AbsPosY
      {
         get
         {
            if (this != MainVm.EditedCpVm)
            {
               if ((TreeParent != null) && (TreeParent is ChildObjectViewModel))
               {
                  ChildObjectViewModel covm = TreeParent as ChildObjectViewModel;

                  return covm.AbsPosY;
               }
            }

            //if (this != MainVm.EditedCpVm)
            //{
            //   double parentPos = 0;

            //   if (ParentVm != null)
            //   {
            //      parentPos = ParentVm.AbsPosY;
            //   }

            //   return parentPos + PosY;
            //}

            return 0;
         }
      }

      public Rect BoundingBox
      {
         get
         {
            // TODO: Systems take up space on a object, but they are not
            // part of the BoundingBox calculation. They assume they are
            // but they are not. Add them here

            BoundingBoxRect bbr = new BoundingBoxRect();

            if (ShapeCollection.Shapes.Count > 0)
            {
               foreach (object o in ShapeCollection.Shapes)
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

            if (ChildObjectCollection.Children.Count > 0)
            {
               foreach (ChildObjectViewModel child in ChildObjectCollection.Children)
               {
                  Rect cb = child.CompObj.BoundingBox;
                  cb.Offset(new Vector(child.PosX, child.PosY));
                  bbr.AddRect(cb);
               }
            }

            if (ImportedObjects.Children.Count > 0)
            {
               foreach (CompoundObjectViewModel covm in ImportedObjects.Children)
               {
                  Rect cb = covm.BoundingBox;
                  bbr.AddRect(cb);
               }
            }

            if (bbr.BoundingBox.IsEmpty)
            {
               return new Rect(0, 0, 100, 100);
            }

            return bbr.BoundingBox;
         }
      }

      public uint AggregatedZLevel
      {
         get
         {
            if (Behaviour.Type == "parallaxBackground")
            {
               return 0;

            }



            uint avgZL = 0;
            uint i = 0;

            foreach (object o in ShapeCollection.Shapes)
            {
               if (o is LfShapeViewModel)
               {
                  LfShapeViewModel shape = (LfShapeViewModel)o;

                  avgZL += shape.ZLevel;
                  i++;
               }
            }

            if (i > 0)
            {
               return avgZL / i;
            }

            return 0;
         }
      }
      #endregion

      #region Private Methods

      private ShapeCollectionViewModel SetShapes(CompoundObject co, bool enabledChildren = true)
      {
         // Create a new collection, forgetting the old one.
         ShapeCollectionViewModel shapes = new ShapeCollectionViewModel(this, this, MainVm, enabledChildren);

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

      private JointCollectionViewModel SetJoints(CompoundObject co, bool enabledChildren = true)
      {
         // Create a new collection, forgetting the old one.
         JointCollectionViewModel joints = new JointCollectionViewModel(this, this, MainVm, enabledChildren);

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

      private SystemCollectionViewModel SetSystems(CompoundObject co, bool enabledChildren = true)
      {
         // Create a new collection, forgetting the old one.
         SystemCollectionViewModel systems = new SystemCollectionViewModel(this, this, MainVm, enabledChildren);

         foreach (CoSystem s in co.Systems)
         {
            CoSystemViewModel svm = new CoSystemViewModel(systems, this, MainVm, s, enabledChildren);
            systems.Systems.Add(svm);
         }

         return systems;
      }


      // All children CompoundObjectViewModels are created here. The constructor of the ChildObjectViewModel
      // creates a ChildObjectStatePropertiesViewModel which's constructor creates the CompoundObjectViewModel.
      private ChildCollectionViewModel SetChildren(CompoundObject ModelObject, bool enabledChildren = true)
      {
         ChildCollectionViewModel schcvm = new ChildCollectionViewModel(this, this, MainVm, enabledChildren);

         foreach (ChildObject cho in ModelObject.ChildObjects)
         {
            ChildObjectViewModel chovm = new ChildObjectViewModel(schcvm, this, MainVm, cho, enabledChildren);

            schcvm.Children.Add(chovm);
         }

         return schcvm;
      }


      private bool AmIAnImportedFile()
      // This method returns true if this object is the root of a imported file.
      // It is exposed on the IsImportedFile attribute publiclly. It is also used
      // internally for special handling where needed. For instance, when 
      // commanded to generate the global shape collection, it relays the command
      // to the top-level CO (that is not an imported file).
      {
         if (ParentVm == null)
         {
            return false;
         }

         return ParentVm.IsObjectImported(this);
      }

      private CompoundObjectViewModel TopParentOfImporedFiles()
      // This method returns true if this is not a imported files.
      // If it is an imported file, it calls the same method recursively
      // on its ParentVm. Eventually, the top-most parent that is not an
      // Imported file will be returned. When the global shape collection 
      // is to be built, this is the parent that shall build the collection.
      {
         if (!AmIAnImportedFile())
         {
            return this;
         }

         if (ParentVm != null)
         {
            return ParentVm.TopParentOfImporedFiles();
         }

         return this;
      }

      #endregion

      #region Public Methods

      public void SetBehaviourPropertyInTreeView()
      {
         // Remove the current element (if any) in the TreeCollection that is of BehaviourViewModelBase
         foreach (CollectionViewModelBase s in TreeCollection)
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
      // The _treeCollection is a collection of all specific object-type
      // collections used to display a headline for all collections in the 
      // tree view.
      {
         _treeCollection.Clear();

         SetBehaviourPropertyInTreeView();

         _treeCollection.Add(ShapeCollection);
         _treeCollection.Add(JointCollection);
         _treeCollection.Add(SystemCollection);
         _treeCollection.Add(ChildObjectCollection);
         _treeCollection.Add(ImportedObjects);
      }

      public LfShapeViewModel AddShape(LeftClickState shapeType, Point position)
      {
         LfShape newShape = null;
         LfShapeViewModel newShapeVm = null;


         if (shapeType == LeftClickState.staticBox)
         {
            newShape = new LfStaticBox();
            newShapeVm = new LfStaticBoxViewModel(ShapeCollection, this, MainVm, (LfStaticBox)newShape);
            ModelObject.StaticBoxes.Add((LfStaticBox)newShape);
         }
         else if (shapeType == LeftClickState.dynamicBox)
         {
            newShape = new LfDynamicBox();
            newShapeVm = new LfDynamicBoxViewModel(ShapeCollection, this, MainVm, (LfDynamicBox)newShape);
            ModelObject.DynamicBoxes.Add((LfDynamicBox)newShape);
         }
         else if (shapeType == LeftClickState.staticCircle)
         {
            newShape = new LfStaticCircle();
            newShapeVm = new LfStaticCircleViewModel(ShapeCollection, this, MainVm, (LfStaticCircle)newShape);
            ModelObject.StaticCircles.Add((LfStaticCircle)newShape);
         }
         else if (shapeType == LeftClickState.dynamicCircle)
         {
            newShape = new LfDynamicCircle();
            newShapeVm = new LfDynamicCircleViewModel(ShapeCollection, this, MainVm, (LfDynamicCircle)newShape);
            ModelObject.DynamicCircles.Add((LfDynamicCircle)newShape);
         }
         else if (shapeType == LeftClickState.spriteBox)
         {
            newShape = new LfSpriteBox();
            newShapeVm = new LfSpriteBoxViewModel(ShapeCollection, this, MainVm, (LfSpriteBox)newShape);
            ModelObject.SpriteBoxes.Add((LfSpriteBox)newShape);
         }
         else if (shapeType == LeftClickState.staticPolygon)
         {
            newShape = new LfStaticPolygon();
            newShapeVm = new LfStaticPolygonViewModel(ShapeCollection, this, MainVm, (LfStaticPolygon)newShape);
            ModelObject.StaticPolygons.Add((LfStaticPolygon)newShape);

         }
         else if (shapeType == LeftClickState.dynamicPolygon)
         {
            newShape = new LfDynamicPolygon();
            newShapeVm = new LfDynamicPolygonViewModel(ShapeCollection, this, MainVm, (LfDynamicPolygon)newShape);
            ModelObject.DynamicPolygons.Add((LfDynamicPolygon)newShape);
         }
         else if (shapeType == LeftClickState.spritePolygon)
         {
            newShape = new LfSpritePolygon();
            newShapeVm = new LfSpritePolygonViewModel(ShapeCollection, this, MainVm, (LfSpritePolygon)newShape);
            ModelObject.SpritePolygons.Add((LfSpritePolygon)newShape);
         }
         else if (shapeType == LeftClickState.staticBoxedSpritePolygon)
         {
            newShape = new LfStaticBoxedSpritePolygon();
            newShapeVm = new LfStaticBoxedSpritePolygonViewModel(ShapeCollection, this, MainVm, (LfStaticBoxedSpritePolygon)newShape);
            ModelObject.StaticBoxedSpritePolygons.Add((LfStaticBoxedSpritePolygon)newShape);
         }
         else if (shapeType == LeftClickState.dynamicBoxedSpritePolygon)
         {
            newShape = new LfDynamicBoxedSpritePolygon();
            newShapeVm = new LfDynamicBoxedSpritePolygonViewModel(ShapeCollection, this, MainVm, (LfDynamicBoxedSpritePolygon)newShape);
            ModelObject.DynamicBoxedSpritePolygons.Add((LfDynamicBoxedSpritePolygon)newShape);
         }

         if (newShapeVm != null)
         {
            newShapeVm.PosX = position.X;
            newShapeVm.PosY = position.Y;
         }

         ShapeCollection.Shapes.Add(newShapeVm);
         
         return newShapeVm;

      }

      public void RemoveShape(LfShapeViewModel svm)
      {
         // Check if there are any joints connected to this svm, if so, removed them
         // We may remove joints so we need a for loop here:
         for (int i = JointCollection.Joints.Count - 1; i >= 0; i--)
         {
            // Below will take care of all joints since they
            // all inherit from WeldJoint
            if (JointCollection.Joints[i] is WeldJointViewModel)
            {
               WeldJointViewModel joint = (WeldJointViewModel)JointCollection.Joints[i];

               if ((joint.AName == svm.Name) || (joint.BName == svm.Name))
               {
                  // Remove the joint
                  ModelObject.RemoveJoint(joint.ModelObject);
                  JointCollection.Joints.RemoveAt(i);
               }
            }
         }

         // Check if there are any systems connected to this svm, if so, removed them
         // We may remove systems so we need a for loop here:
         for (int i = SystemCollection.Systems.Count - 1; i >= 0; i--)
         {
            if (SystemCollection.Systems[i] is CoSystemViewModel)
            {
               CoSystemViewModel system = (CoSystemViewModel)SystemCollection.Systems[i];

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
                  SystemCollection.Systems.RemoveAt(i);
               }
            }
         }

         // Remove the shape model
         ModelObject.RemoveShape(svm.ModelObject);

         // Remove the shape viewmodel from this
         ShapeCollection.Shapes.Remove(svm);

         // If there are no more shapes in the CO, remove the CO
         if (ShapeCollection.Shapes.Count == 0)
         {
            //ParentVm.StateChildObjects.Remove(this);
            //ParentVm.ModelObject.ChildObjectRefs(this.ChildObjectOfParent)
         }

         OnPropertyChanged("");
      }

      public WeldJointViewModel AddJoint(
         LeftClickState shapeType, 
         Point position, 
         string shapeAName, 
         string shapeBName, 
         int numOfShapes)
      {

         // First do special handling for ropes and later handle 
         // all other joints
         if (shapeType == LeftClickState.rope)
         {
            Rope rp = new Rope();
            RopeViewModel rpvm = new RopeViewModel(JointCollection, this, MainVm, rp);

            rp.AName = shapeAName;

            if (numOfShapes == 2)
            {
               rp.BName = shapeBName;
            }

            // If there is no shape B, the rp.BName should be default "notDef"

            rpvm.ConnectToShapes(ShapeCollection.Shapes);

            Point parentObjectOrigo = new Point(0, 0);

            // Shape A point
            Point shapeAOrigo = new Point(rpvm.AShapeObject.PosX, rpvm.AShapeObject.PosY);
            shapeAOrigo.Offset(parentObjectOrigo.X, parentObjectOrigo.Y);
            Point localAClickPoint = new Point();
            localAClickPoint = (Point)(position - shapeAOrigo);

            // Rotate point to shape rotation
            Point rotatedAClickPoint = CoordinateTransformations.LocalPointFromRotated(localAClickPoint, rpvm.AShapeObject.Angle);

            rpvm.AAnchorX = rotatedAClickPoint.X;
            rpvm.AAnchorY = rotatedAClickPoint.Y;

            if (numOfShapes == 2)
            {
               // Shape B point
               Point shapeBOrigo = new Point(rpvm.BShapeObject.PosX, rpvm.BShapeObject.PosY);
               shapeBOrigo.Offset(parentObjectOrigo.X, parentObjectOrigo.Y);
               Point localBClickPoint = new Point();
               localBClickPoint = (Point)(position - shapeBOrigo);

               // Rotate point to shape rotation
               Point rotatedBClickPoint = CoordinateTransformations.LocalPointFromRotated(localBClickPoint, rpvm.BShapeObject.Angle);

               rpvm.BAnchorX = rotatedBClickPoint.X;
               rpvm.BAnchorY = rotatedBClickPoint.Y;
            }

            JointCollection.Joints.Add(rpvm);
            ModelObject.Ropes.Add(rp);

            return rpvm;
         }
         else
         {
            // If it wasn't a rope, it was some other joint...
            WeldJoint wj = null;
            WeldJointViewModel wjvm = null;


            if (shapeType == LeftClickState.weldJoint)
            {
               wj = new WeldJoint();
               wj.AName = shapeAName;
               wj.BName = shapeBName;
               wjvm = new WeldJointViewModel(JointCollection, this, MainVm, wj);
               ModelObject.WeldJoints.Add(wjvm.ModelObject);
            }
            else if (shapeType == LeftClickState.revoluteJoint)
            {
               wj = new RevoluteJoint();
               wj.AName = shapeAName;
               wj.BName = shapeBName;
               wjvm = new RevoluteJointViewModel(JointCollection, this, MainVm, (RevoluteJoint)wj);
               ModelObject.RevoluteJoints.Add((RevoluteJoint)wjvm.ModelObject);
            }
            else if (shapeType == LeftClickState.prismaticJoint)
            {
               wj = new PrismaticJoint();
               wj.AName = shapeAName;
               wj.BName = shapeBName;
               wjvm = new PrismaticJointViewModel(JointCollection, this, MainVm, (PrismaticJoint)wj);
               ModelObject.PrismaticJoints.Add((PrismaticJoint)wjvm.ModelObject);
            }

            wjvm.ConnectToShapes(ShapeCollection.Shapes);

            Point parentObjectOrigo = new Point(0, 0);

            // Shape A point
            Point shapeAOrigo = new Point(wjvm.AShapeObject.PosX, wjvm.AShapeObject.PosY);
            shapeAOrigo.Offset(parentObjectOrigo.X, parentObjectOrigo.Y);

            Point localAClickPoint = new Point();
            localAClickPoint = (Point)(position - shapeAOrigo);


            // Rotate point to shape rotation
            Point rotatedAClickPoint = CoordinateTransformations.LocalPointFromRotated(localAClickPoint, wjvm.AShapeObject.Angle);

            wjvm.AAnchorX = rotatedAClickPoint.X;
            wjvm.AAnchorY = rotatedAClickPoint.Y;

            // Shape B point
            Point shapeBOrigo = new Point(wjvm.BShapeObject.PosX, wjvm.BShapeObject.PosY);
            shapeBOrigo.Offset(parentObjectOrigo.X, parentObjectOrigo.Y);
            Point localBClickPoint = new Point();
            localBClickPoint = (Point)(position - shapeBOrigo);

            // Rotate point to shape rotation
            //               Point rotatedBClickPoint = wjvm.BShapeObject.RotatedPointFromLocal(localBClickPoint);
            Point rotatedBClickPoint = CoordinateTransformations.LocalPointFromRotated(localBClickPoint, wjvm.BShapeObject.Angle);

            wjvm.BAnchorX = rotatedBClickPoint.X;
            wjvm.BAnchorY = rotatedBClickPoint.Y;

            JointCollection.Joints.Add(wjvm);

            return wjvm;
         }
      }


      public CoSystemViewModel AddSystem(
         LeftClickState shapeType, 
         Point position, 
         string shapeName,
         int numOfShapes)
      {
         CoSystem newSystem = new CoSystem();
         CoSystemViewModel newSysVm = null;

         Point parentObjectOrigo = new Point(0, 0);

         if (shapeType == LeftClickState.objectFactory)
         {
            newSystem.Type = "objectFactory";
            newSystem.ObjFactStateProperties = new ObjectFactoryProperties();

            if (numOfShapes == 1)
            {
               newSystem.ObjFactStateProperties.Body = shapeName;
            }
            else
            {
               newSystem.ObjFactStateProperties.Body = "notDefined";
            }

            newSysVm = new CoSystemViewModel(
               SystemCollection,
               this,
               MainVm,
               newSystem,
               true);

            ObjectFactoryPropertiesViewModel propVm = newSysVm.Properties as ObjectFactoryPropertiesViewModel;

            if (numOfShapes == 1)
            {
               parentObjectOrigo = new Point(propVm.BodyObject.PosX, propVm.BodyObject.PosY);
            }

            // Shape point
            Point shapeAOrigo = new Point(propVm.PosX, propVm.PosY);
            shapeAOrigo.Offset(parentObjectOrigo.X, parentObjectOrigo.Y);
            Point localAClickPoint = new Point();
            localAClickPoint = (Point)(position - shapeAOrigo);

            // Rotate point to shape rotation
            if (numOfShapes > 0)
            {
               Point rotatedAClickPoint = CoordinateTransformations.LocalPointFromRotated(localAClickPoint, propVm.BodyObject.Angle);

               propVm.PosX = rotatedAClickPoint.X;
               propVm.PosY = rotatedAClickPoint.Y;
            }
            else
            {
               propVm.PosX = localAClickPoint.X;
               propVm.PosY = localAClickPoint.Y;
            }

         }
         else if (shapeType == LeftClickState.reentryFlameEmitter)
         {
            newSystem.Type = shapeType.ToString();
            newSystem.ReentryFlameEmitterStateProperties = new ReentryFlameEmitterProperties();
            newSystem.ReentryFlameEmitterStateProperties.BodyName = shapeName;

            newSysVm = new CoSystemViewModel(
               SystemCollection,
               this,
               MainVm,
               newSystem,
               true);

            ReentryFlameEmitterPropertiesViewModel propVm = newSysVm.Properties as ReentryFlameEmitterPropertiesViewModel;

            propVm.ConnectToShapes(ShapeCollection.Shapes);

            parentObjectOrigo = new Point(propVm.BodyObject.PosX, propVm.BodyObject.PosY);

            // Shape point
            Point shapeAOrigo = new Point(propVm.EmitterLineStartX, propVm.EmitterLineStartY);
            shapeAOrigo.Offset(parentObjectOrigo.X, parentObjectOrigo.Y);
            Point localAClickPoint = new Point();
            localAClickPoint = (Point)(position - shapeAOrigo);

            // Rotate point to shape rotation
            Point rotatedAClickPoint = CoordinateTransformations.LocalPointFromRotated(localAClickPoint, propVm.BodyObject.Angle);

            propVm.EmitterLineStartX = rotatedAClickPoint.X;
            propVm.EmitterLineStartY = rotatedAClickPoint.Y;

            propVm.EmitterLineEndX = propVm.EmitterLineStartX + 10;
            propVm.EmitterLineEndY = propVm.EmitterLineStartY + 10;

         }
         else
         {
            newSystem.Type = shapeType.ToString();

            if (shapeType == LeftClickState.gun)
            {
               newSystem.GunStateProperties = new GunProperties();
               newSystem.GunStateProperties.BodyName = shapeName;
            }
            else if (shapeType == LeftClickState.flameEmitter)
            {
               newSystem.FlameEmitterStateProperties = new FlameEmitterProperties();
               newSystem.FlameEmitterStateProperties.BodyName = shapeName;
            }
            else if (shapeType == LeftClickState.shield)
            {
               newSystem.ShieldStateProperties = new ShieldProperties();
               newSystem.ShieldStateProperties.BodyName = shapeName;
            }

            newSysVm = new CoSystemViewModel(
               SystemCollection,
               this,
               MainVm,
               newSystem,
               true);

            BodyOriginSystemViewModel propVm = newSysVm.Properties as BodyOriginSystemViewModel;

            propVm.ConnectToShapes(ShapeCollection.Shapes);

            parentObjectOrigo = new Point(propVm.BodyObject.PosX, propVm.BodyObject.PosY);

            parentObjectOrigo = new Point(propVm.BodyObject.PosX, propVm.BodyObject.PosY);
            
            // Shape point
            Point shapeAOrigo = new Point(propVm.SystemOriginX, propVm.SystemOriginY);
            shapeAOrigo.Offset(parentObjectOrigo.X, parentObjectOrigo.Y);
            Point localAClickPoint = new Point();
            localAClickPoint = (Point)(position - shapeAOrigo);

            // Rotate point to shape rotation
            Point rotatedAClickPoint = CoordinateTransformations.LocalPointFromRotated(localAClickPoint, propVm.BodyObject.Angle);

            propVm.SystemOriginX = rotatedAClickPoint.X;
            propVm.SystemOriginY = rotatedAClickPoint.Y;

         }

         SystemCollection.Systems.Add(newSysVm);
         ModelObject.Systems.Add(newSystem);

         return newSysVm;
      }

      public void InvalidateJoints()
      {
         foreach (object o in JointCollection.Joints)
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
         foreach (ChildObjectViewModel chvm in ChildObjectCollection.Children)
         {
            chvm.CompObj.OnPropertyChanged("");
         }
      }

      public void BuildViewModel(bool enabledChildren = true)
      // This method creates view model objects for all model objects in the collection
      // of the ModelObject instance of this view model. It then connects references in the 
      // COVM. At the creation of the COVM the ModelObject, ModelObjectProperties, ParentVm 
      // and ChildObjectOfParent is defined.
      // Here we build the shape, joint and system collections and connect those objects 
      // with the necessary references. The ChildObjects are also added to the corresponding
      // state index in the StateChildObjects collection
      // TODO: Here the imported objects (files) need to be handled. 
      {
         // First create viewmodels for all model object
         _shapes = SetShapes(ModelObject, enabledChildren);
         _joints = SetJoints(ModelObject, enabledChildren);
         _systems = SetSystems(ModelObject, enabledChildren);

         ChildObjectCollection = SetChildren(ModelObject, enabledChildren);


         // The TreeViewCollection holds the same objects of shapes, joints, systems
         // etc, but in a hierarcichal structure of the tree view. 
         BuildTreeViewCollection();
      }

      public void BuildGlobalShapeCollection(bool showJoints, bool showSystems, bool showBackgrounds)
      {
         if (AmIAnImportedFile())
         {
            TopParentOfImporedFiles().BuildGlobalShapeCollection(showJoints, showSystems, showBackgrounds);
            return;
         }

         GlobalShapeCollection.Clear();

         AddShapesToGlobalCollection(ref _globalShapeCollection, showJoints, showSystems, showBackgrounds);

         ConnectNamedObjects(_globalShapeCollection);
      }

      public void AddShapesToGlobalCollection(ref CompositeCollection coll, bool showJoints, bool showSystems, bool showBackgrounds)
      {
         foreach (Object o in ShapeCollection.Shapes)
         {
            if (o is LfShapeViewModel)
            {
               LfShapeViewModel svm = o as LfShapeViewModel;
               coll.Add(svm);
            }
         }

         if (showJoints)
         {
            foreach (Object o in JointCollection.Joints)
            {
               if (o is WeldJointViewModel)
               {
                  WeldJointViewModel wjvm = o as WeldJointViewModel;
                  coll.Add(wjvm);
               }
            }
         }

         if (showSystems)
         {
            foreach (Object o in SystemCollection.Systems)
            {
               if (o is CoSystemViewModel)
               {
                  CoSystemViewModel svm = o as CoSystemViewModel;
                  coll.Add(svm);
               }
            }
         }

         foreach (ChildObjectViewModel covm in ChildObjectCollection.Children)
         {
            covm.AddChildShapesToGlobalCollection(ref coll, showJoints, showSystems, showBackgrounds);
         }

         foreach (CompoundObjectViewModel covm in ImportedObjects.Children)
         {
            covm.AddShapesToGlobalCollection(ref coll, showJoints, showSystems, showBackgrounds);
         }
      }

      public void ConnectNamedObjects(CompositeCollection coll)
      {
         // Only now is the Joints property valid for this state
         // Connect joints to shape, creating the reference link
         // that is defined by body names in xml file.
         foreach (object o in coll)
         {
            if (o is WeldJointViewModel)
            {
               if (o is RopeViewModel)
               {
                  RopeViewModel joint = (RopeViewModel)o;

                  joint.ConnectToShapes(coll);
               }
               else
               {
                  WeldJointViewModel joint = (WeldJointViewModel)o;

                  joint.ConnectToShapes(coll);
               }
            }
         }

         // Only now is the Systems property valid for this state
         // Connect systems to shapes, creating the reference link
         // that is defined by body names in xml file.
         foreach (object o in coll)
         {
            if (o is CoSystemViewModel)
            {
               CoSystemViewModel sys = o as CoSystemViewModel;

               if (sys.Properties is BodyOriginSystemViewModel)
               {
                  BodyOriginSystemViewModel sysProp = sys.Properties as BodyOriginSystemViewModel;
                  sysProp.ConnectToShapes(coll);
               }

               if (sys.Properties is ReentryFlameEmitterPropertiesViewModel)
               {
                  ReentryFlameEmitterPropertiesViewModel sysProp = sys.Properties as ReentryFlameEmitterPropertiesViewModel;
                  sysProp.ConnectToShapes(coll);
               }
            }
         }


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

                     sovm.SpawnChildObject[0].CompObj.IsSelected = false;
                  }
               }
            }
         }

         if ((ShapeCollection != null) && (ShapeCollection.Shapes != null))
         {
            foreach (object o in ShapeCollection.Shapes)
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

         if (JointCollection != null)
         {
            foreach (object o in JointCollection.Joints)
            {
               if (o is WeldJointViewModel)
               {
                  WeldJointViewModel joint = (WeldJointViewModel)o;

                  joint.IsSelected = false;
               }
            }
         }

         if (SystemCollection != null)
         {
            foreach (object o in SystemCollection.Systems)
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

         if (ChildObjectCollection != null)
         {
            foreach (ChildObjectViewModel child in ChildObjectCollection.Children)
            {
               child.DeselectAllChildren();
               child.IsSelected = false;
            }
         }
      }


      public bool ChildHasFileReference(string fileName)
      {
         foreach (ChildObjectViewModel chovm in ChildObjectCollection.Children)
         {
            if (chovm.IsFileReferenceChild)
            {
               return true;
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


      public void GenerateTriangles()
      {
         foreach (object o in ShapeCollection.Shapes)
         {
            if (o is LfPolygonViewModel)
            {
               LfPolygonViewModel pvm = (LfPolygonViewModel)o;

               pvm.GenerateTriangles();
            }
         }

         foreach (ChildObjectViewModel covm in ChildObjectCollection.Children)
         {
            covm.CompObj.GenerateTriangles();
         }
      }

      public void InvalidateSystemProperties()
      {
         foreach (object o in SystemCollection.Systems)
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

      public LfShapeViewModel FindBodyObject(string bodyName)
      {
         foreach (object o in ShapeCollection.Shapes)
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

      public LfDragablePointViewModel AddPoint(LfPolygonViewModel polyVm, Point position)
      {
         LfPolygonViewModel newPolygon = polyVm;
         Point parentObjectOrigo = new Point(newPolygon.ParentVm.PosX, newPolygon.ParentVm.PosY);
         Point shapeOrigo = new Point(newPolygon.PosX, newPolygon.PosY);
         shapeOrigo.Offset(parentObjectOrigo.X, parentObjectOrigo.Y);
         Point localClickPoint = new Point();
         localClickPoint = (Point)(position - shapeOrigo);

         LfDragablePointViewModel newPoint = newPolygon.AddPoint(localClickPoint);

         return newPoint;
      }

      public LfShapeViewModel FindShape(string name, CompositeCollection objects)
      {
         foreach (object o in objects)
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


      // This method returns with the supplied point in the scene's coordinate system
      // (Equal to the top level CompoundObject's coordinate system). 
      // The point is converted to the parent's coordinate system and the method is then
      // recursively called for the parent until the parent is null. In this case we are 
      // at the top level CompoundObject which is the scene. Then we returns the point
      public Point GetScenePointFromCoPoint(Point coPoint)
      {
         if ((ParentVm != null) && (ParentVm is CompoundObjectViewModel))
         {
            CompoundObjectViewModel covm = ParentVm as CompoundObjectViewModel;
            Point parentPoint = covm.ParentCoPoint(coPoint);
            return GetScenePointFromCoPoint(parentPoint);
         }
         else
         {
            return coPoint;
         }
      }

      public virtual Point GetCoPointFromScenePoint(Point scenePoint)
      {
         if ((ParentVm != null) && (ParentVm is CompoundObjectViewModel))
         {
            if (ParentVm.IsObjectMyChild(this))
            {
               Point parentPoint = ParentVm.GetCoPointFromScenePoint(scenePoint);
               return GetScenePointFromCoPoint(parentPoint);
            }
         }

         return scenePoint;
      }

      public void UpdateChildrenAbsolutePos()
      {
         foreach (Object o in ShapeCollection.Shapes)
         {
            if (o is LfShapeViewModel)
            {
               LfShapeViewModel svm = o as LfShapeViewModel;
               svm.OnPropertyChanged("AbsPosX");
               svm.OnPropertyChanged("AbsPosY");
            }
         }

         foreach (Object o in JointCollection.Joints)
         {
            if (o is WeldJointViewModel)
            {
               WeldJointViewModel wjvm = o as WeldJointViewModel;
               wjvm.OnPropertyChanged("AbsPosX");
               wjvm.OnPropertyChanged("AbsPosY");
            }
         }

         // TODO: Why not systems??

         foreach (ChildObjectViewModel covm in ChildObjectCollection.Children)
         {
            covm.UpdateChildrenAbsolutePos();
         }
      }

      public bool IsObjectImported(CompoundObjectViewModel covm)
      // This method returns true if covm is part of the ImportedObjects collection
      // Otherwise, it returns false. If the object is a child, this is evidence that
      // covm is a ImportedObject. 
      {
         foreach (CompoundObjectViewModel covmit in _importedObjects.Children)
         {
            if (covm == covmit)
            {
               return true;
            }
         }

         return false;
      }

      public bool IsObjectMyChild(CompoundObjectViewModel covm)
      // This method returns true if covm is part of the ChildObjectCollection
      // Otherwise, it returns false. If the object is a child, this is evidence that
      // covm is a ImportedObject. 
      {
         foreach (ChildObjectViewModel covmit in ChildObjectCollection.Children)
         {
            if (covm == covmit.CompObj)
            {
               return true;
            }
         }

         return false;
      }

      #endregion

   }
}
