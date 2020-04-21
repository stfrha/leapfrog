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
 * The ChildCOViewModel encompass the CompoundObject (incl its behaviour) 
 * but provides a way to add extra behaviour. The CHildCOViewModel can 
 * relate to a ChildObject that is either a file reference or a child object
 * that is a CompoundObject defined in the parenting XML-file. 
 */


namespace LeapfrogEditor
{   
   public class ChildCOViewModel : CompoundObjectViewModel, IPositionInterface
   {
      #region Declarations

      //private ObservableCollection<CollectionViewModelBase> _treeCollection = new ObservableCollection<CollectionViewModelBase>();

      #endregion

      #region Constructors

      public ChildCOViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         CompoundObject compoundObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, compoundObject)
      {
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

      override public string Name
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
            string fileRef = "";

            if ((File != "") && (File != "undef_file.xml"))
            {
               fileRef = " - " + File;
            }

            return Name + " - " + fileRef;
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

      public string File
      {
         get
         {
            if ((TreeParent != null) && (TreeParent is ChildObjectViewModel))
            {
               ChildObjectViewModel covm = TreeParent as ChildObjectViewModel;

               return covm.File;
            }

            return "Error resolving file name";
         }
         set
         {
            if ((TreeParent != null) && (TreeParent is ChildObjectViewModel))
            {
               ChildObjectViewModel covm = TreeParent as ChildObjectViewModel;

               covm.File = value;
               OnPropertyChanged("File");
               OnPropertyChanged("DispName");
               OnPropertyChanged("IsFileReferenceChild");
               covm.OnPropertyChanged("File");
            }
         }
      }

      public new double PosX
      {
         get
         {
            if ((TreeParent != null) && (TreeParent is ChildObjectViewModel))
            {
               ChildObjectViewModel covm = TreeParent as ChildObjectViewModel;
               return covm.PosX;
            }

            return 0;

         }
         set
         {
            if ((TreeParent != null) && (TreeParent is ChildObjectViewModel))
            {
               ChildObjectViewModel covm = TreeParent as ChildObjectViewModel;
               covm.PosX = value;

               OnPropertyChanged("PosX");
               OnPropertyChanged("AbsPosX");
               OnPropertyChanged("BoundingBox");

               CompoundObjectViewModel p = ParentVm;

               while ((p != null) && (p is ChildCOViewModel))
               {
                  p.OnPropertyChanged("BoundingBox");
                  p = p.ParentVm;
               }

               UpdateChildrenAbsolutePos();
            }

         }
      }

      public new double PosY
      {
         get
         {
            if ((TreeParent != null) && (TreeParent is ChildObjectViewModel))
            {
               ChildObjectViewModel covm = TreeParent as ChildObjectViewModel;
               return covm.PosY;
            }

            return 0;
         }
         set
         {
            if ((TreeParent != null) && (TreeParent is ChildObjectViewModel))
            {
               ChildObjectViewModel covm = TreeParent as ChildObjectViewModel;
               covm.PosY = value;

               OnPropertyChanged("PosY");
               OnPropertyChanged("AbsPosY");
               OnPropertyChanged("BoundingBox");

               CompoundObjectViewModel p = ParentVm;

               while ((p != null) && (p is ChildCOViewModel))
               {
                  p.OnPropertyChanged("BoundingBox");
                  p = p.ParentVm;
               }

               UpdateChildrenAbsolutePos();
            }
         }
      }

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
