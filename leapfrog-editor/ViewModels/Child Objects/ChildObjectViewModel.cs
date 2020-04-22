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
 * The ChildObjectViewModel encompass the CompoundObject (incl its states, and behaviour) 
 * as well as all the ChildObjects of all states
 */


namespace LeapfrogEditor
{
   // Used to be ConditionalSelectTreeViewViewModel
   public class ChildObjectViewModel : TreeViewViewModel, IPositionInterface
   {
      #region Declarations

      private ChildObject _modelObject;
      private ChildCOViewModel _compObj;
      private ObservableCollection<ChildCOViewModel> _co = new ObservableCollection<ChildCOViewModel>();
      
      #endregion

      #region Constructors

      public ChildObjectViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm, 
         ChildObject modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;

         CompObj = new ChildCOViewModel(this, parentVm, mainVm, _modelObject.CompObj, enabled);
         _co.Clear();
         _co.Add(CompObj);

         if (IsFileReferenceChild)
         {
            CompObj.BuildViewModel(false);
         }
         else
         {
            CompObj.BuildViewModel(true);
         }
      }

      #endregion

      #region Properties

      public ChildObject ModelObject
      {
         get { return _modelObject; }
         set
         {
            _modelObject = value;
            OnPropertyChanged("");
         }
      }

      public ObservableCollection<ChildCOViewModel> Co
      {
         get { return _co; }
         set { _co = value; }
      }

      public string Name
      {
         get { return _modelObject.Name; }
         set
         {
            _modelObject.Name = value;
            OnPropertyChanged("Name");
            OnPropertyChanged("RefName");
            if (TreeParent != null)
            {
               TreeParent.OnPropertyChanged("");
            }
         }
      }

      public string File
      {
         get { return _modelObject.File; }
         set
         {
            _modelObject.File = value;
            OnPropertyChanged("File");
            OnPropertyChanged("DispName");
            OnPropertyChanged("IsFileReferenceChild");
         }
      }

      public double PosX
      {
         get
         {
            return _modelObject.PosX;
         }
         set
         {
            _modelObject.PosX = value;

            OnPropertyChanged("PosX");
            OnPropertyChanged("AbsPosX");
            OnPropertyChanged("BoundingBox");

            CompObj.OnPropertyChanged("AbsPosX");

            CompoundObjectViewModel p = ParentVm;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p = p.ParentVm;
            }

            UpdateChildrenAbsolutePos();
         }
      }

      public double AbsPosX
      {
         get
         {
            double parentPos = 0;

            if (ParentVm != null)
            {
               parentPos = ParentVm.AbsPosX;
            }

            return parentPos + PosX;
         }
      }


      public double PosY
      {
         get
         {
            return _modelObject.PosY;
         }
         set
         {
            _modelObject.PosY = value;

            OnPropertyChanged("PosY");
            OnPropertyChanged("AbsPosY");
            OnPropertyChanged("BoundingBox");

            CompObj.OnPropertyChanged("AbsPosY");

            CompoundObjectViewModel p = ParentVm;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p = p.ParentVm;
            }

            UpdateChildrenAbsolutePos();
         }
      }

      public double AbsPosY
      {
         get
         {
            double parentPos = 0;

            if (ParentVm != null)
            {
               parentPos = ParentVm.AbsPosY;
            }

            return parentPos + PosY;
         }
      }


      public ChildCOViewModel CompObj
      {
         get
         {
            return _compObj;
         }
         set
         {
            _compObj = value;
            _co.Clear();
            _co.Add(_compObj);
            OnPropertyChanged("CompObj");
         }
      }

      public bool IsFileReferenceChild
      {
         get
         {
            if ((File != "") && (File != "undef_file.xml"))
            {
               return true;
            }

            return false;
         }
      }

      public uint AggregatedZLevel
      {
         get
         {
            return CompObj.AggregatedZLevel;

         }
      }

      #endregion

      #region Private Methods


      #endregion

      #region Public Methods


      public void DeselectAllChildren()
      {
         CompObj.DeselectAllChildren();
         CompObj.IsSelected = false;
      }

      public void AddChildShapesToGlobalCollection(ref CompositeCollection coll, bool showJoints, bool showSystems, bool showBackgrounds)
      {

         foreach (Object o in CompObj.ShapeCollection.Shapes)
         {
            if (o is LfShapeViewModel)
            {
               LfShapeViewModel svm = o as LfShapeViewModel;
               coll.Add(svm);
            }
         }

         if (showJoints)
         {
            foreach (Object o in CompObj.JointCollection.Joints)
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
            foreach (Object o in CompObj.SystemCollection.Systems)
            {
               if (o is CoSystemViewModel)
               {
                  CoSystemViewModel svm = o as CoSystemViewModel;
                  coll.Add(svm);
               }
            }
         }


         if (CompObj != MainVm.EditedCpVm)
         {
            foreach (ChildObjectViewModel covm in CompObj.ChildObjectCollection.Children)
            {
               covm.AddChildShapesToGlobalCollection(ref coll, showJoints, showSystems, showBackgrounds);
            }
         }
      }

      public void UpdateChildrenAbsolutePos()
      {
         CompObj.UpdateChildrenAbsolutePos();
      }

      // This method returns with the supplied point (expressed in this CompoundObject's
      // coordinate system) converted to the parent CompoundObjects coordinate system.
      // This is done by adding the Position of this CompoundObject.
      public Point ParentCoPoint(Point coPoint)
      {
         Point parentPoint = coPoint;

         parentPoint.Offset(PosX, PosY);

         return parentPoint;
      }

      // This method returns with the supplied point (expressed in the parent of this 
      // CompoundObject's coordinate system) converted to this CompoundObjects coordinate system.
      // This is done by subtracting the parentPoint with the position of this CompoundObject.
      public Point CoPointFromParent(Point parentPoint)
      {
         Point coPoint = parentPoint;

         coPoint.Offset(-PosX, -PosY);

         return coPoint;
      }

      public  Point GetCoPointFromScenePoint(Point scenePoint)
      {
         if (ParentVm != null)
         {
            Point parentPoint = CoPointFromParent(scenePoint);
            return GetCoPointFromScenePoint(parentPoint);
         }
         else
         {
            return scenePoint;
         }
      }


      #endregion

   }
}
