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
 * The ChildCOViewModel encompass the CompoundObject (incl its states, and behaviour) 
 * as well as all the ChildObjects of all states
 */


namespace LeapfrogEditor
{   // used to be: ConditionalSelectTreeViewViewModel
   public class ChildCOViewModel : CompoundObjectViewModel, IPositionInterface
   {
      #region Declarations

      private TStateProperties<ChildObjectStateProperties> _childStateModelObject;

      private int _selectedStateIndex = 0;

      private ObservableCollection<StateCollectionViewModelBase> _treeCollection = new ObservableCollection<StateCollectionViewModelBase>();
      #endregion

      #region Constructors

      public ChildCOViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         TStateProperties<ChildObjectStateProperties> childStateModelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, childStateModelObject.Properties.CompObj)
      {
         ChildStateModelObject = childStateModelObject;

         // Below we need to find the top level behavior. Must get it from the MainViewModel!
         int i = MainVm.GetEditableCoBehaviourIndexOf(this, ChildStateModelObject.State);

         if (i < 0)
         {
            i = 0;
         }

         SelectedStateIndex = i;
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

      public TStateProperties<ChildObjectStateProperties> ChildStateModelObject
      {
         get { return _childStateModelObject; }
         set
         {
            _childStateModelObject = value;
            OnPropertyChanged("");
         }
      }

      public override string Name
      {
         get
         {
            if ((TreeParent != null) && (TreeParent is ChildObjectViewModel))
            {
               ChildObjectViewModel covm = TreeParent as ChildObjectViewModel;

               return covm.Name;
            }

            return "Error: could not resolve name";
         }
         set
         {
            if ((TreeParent != null) && (TreeParent is ChildObjectViewModel))
            {
               ChildObjectViewModel covm = TreeParent as ChildObjectViewModel;

               covm.Name = value;
               OnPropertyChanged("Name");
               OnPropertyChanged("DispName");
               covm.OnPropertyChanged("Name");
            }
         }
      }

      public string DispName
      {
         get
         {
            string stateRef = "";

            stateRef = " - " + State;

            string fileRef = "";

            if ((File != "") && (File != "undef_file.xml"))
            {
               fileRef = " - " + File;
            }

            return Name + stateRef + fileRef;
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

      public string ObjectState
      {
         get
         {
            return State;
         }
      }

      public string State
      {
         get { return ChildStateModelObject.State; }
      }

      public int SelectedStateIndex
      {
         get
         {
            return _selectedStateIndex;
         }
         set
         {
            int prevI = _selectedStateIndex;

            if (prevI == value)
            {
               return;
            }

            if ((value == -1) || (value > ParentVm.Behaviour.States.Count - 1))
            {
               _selectedStateIndex = 0;
            }
            else
            {
               _selectedStateIndex = value;
            }

            ChildStateModelObject.State = MainVm.EditedCpVm.Behaviour.States[_selectedStateIndex].StateName;

            OnPropertyChanged("");

            ParentVm.DeselectAllChildren();



            CompoundObjectViewModel p = ParentVm;

            while ((p != null) && (p is ChildCOViewModel))
            {
               p.OnPropertyChanged("BoundingBox");
               p = p.ParentVm;
            }
         }
      }

      public string File
      {
         get { return ChildStateModelObject.Properties.File; }
         set
         {
            ChildStateModelObject.Properties.File = value;
            OnPropertyChanged("File");
            OnPropertyChanged("DispName");
            OnPropertyChanged("IsFileReferenceChild");
         }
      }

      public new double PosX
      {
         get
         {
            return ChildStateModelObject.Properties.PosX;
         }
         set
         {
            ChildStateModelObject.Properties.PosX = value;

            OnPropertyChanged("PosX");
            OnPropertyChanged("BoundingBox");

            CompoundObjectViewModel p = ParentVm;

            while ((p != null) && (p is ChildCOViewModel))
            {
               p.OnPropertyChanged("BoundingBox");
               p = p.ParentVm;
            }
         }
      }

      public new double PosY
      {
         get
         {
            return ChildStateModelObject.Properties.PosY;
         }
         set
         {
            ChildStateModelObject.Properties.PosY = value;

            OnPropertyChanged("PosY");
            OnPropertyChanged("BoundingBox");

            CompoundObjectViewModel p = ParentVm;

            while ((p != null) && (p is ChildCOViewModel))
            { 
               p.OnPropertyChanged("BoundingBox");
               p = p.ParentVm;
            }
         }
      }

      //public Rect BoundingBox
      //{
      //   get
      //   {
      //      if (StateShapes == null) return new Rect(0, 0, 100, 100);

      //      if ((StateShapes.Shapes.Count == 0) && (ChildObjectsWithStates.Children.Count == 0))
      //      {
      //         return new Rect(0,0,0,0);
      //      }

      //      BoundingBoxRect bbr = new BoundingBoxRect();

      //      if (StateShapes.Shapes.Count > 0)
      //      {
      //         foreach (object o in StateShapes.Shapes)
      //         {
      //            if (o is LfShapeViewModel)
      //            {
      //               LfShapeViewModel shape = (LfShapeViewModel)o;

      //               Rect cb = shape.BoundingBox;
      //               cb.Offset(new Vector(shape.PosX, shape.PosY));
      //               bbr.AddRect(cb);
      //            }
      //         }
      //      }

      //      if (ChildObjectsWithStates.Children.Count > 0)
      //      {
      //         // TODO: Expand this to work with all Displayed ChildObjects, the uncomment
      //         //foreach (ChildCOViewModel child in ChildObjectsWithStates.Children)
      //         //{
      //         //   Rect cb = child.BoundingBox;
      //         //   cb.Offset(new Vector(child.PosX, child.PosY));
      //         //   bbr.AddRect(cb);
      //         //}
      //      }

      //      return bbr.BoundingBox;
      //   }
      //   set
      //   { }
      //}

      #endregion

      #region Private Methods

      #endregion

      #region Public Methods

      // This method returns with the supplied point (expressed in this CompoundObject's
      // coordinate system) converted to the parent CompoundObjects coordinate system.
      // This is done by adding the Position of this CompoundObject.
      public override Point ParentCoPoint(Point coPoint)
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

      public override Point GetCoPointFromScenePoint(Point scenePoint)
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
