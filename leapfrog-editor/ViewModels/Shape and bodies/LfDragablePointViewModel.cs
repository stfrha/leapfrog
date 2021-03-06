﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LeapfrogEditor
{  // used to be MicroMvvm.ViewModelBase
   public class LfDragablePointViewModel : TreeViewViewModel, IPositionInterface
   {
      #region Declarations

      private LfDragablePoint _modelObject;
      private LfPolygonViewModel _parent;


      #endregion

      #region Constructors

      public LfDragablePointViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm, 
         LfPolygonViewModel parent, 
         LfDragablePoint modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         MainVm = mainVm;
         Parent = parent;
         ModelObject = modelObject;
         IsSelected = false;
      }

      #endregion

      #region Properties

      public LfPolygonViewModel Parent
      {
         get { return _parent; }
         set
         {
            _parent = value;
            OnPropertyChanged("");
         }
      }

      public LfDragablePoint ModelObject
      {
         get { return _modelObject; }
         set
         {
            _modelObject = value;
            OnPropertyChanged("");
         }
      }

      public double PosX
      {
         get
         {
            if (ModelObject == null) return 0;

            return ModelObject.PosX;
         }
         set
         {
            if (ModelObject == null) return;

            ModelObject.PosX = value;

            if (Parent != null)
            {
               Parent.OnPropertyChanged("Points");
               Parent.OnPropertyChanged("AbsPoints");
               Parent.OnPropertyChanged("BoundingBox");

               // We must also make sure that the previous point in the collection
               // is updated since the line between them must be redrawn
               int i = Parent.PointVms.IndexOf(this);

               if (i != -1)
               {
                  if (i < Parent.PointVms.Count() - 1)
                  {
                     Parent.PointVms[i + 1].OnPropertyChanged("PosX");
                     Parent.PointVms[i + 1].OnPropertyChanged("AbsPosX");
                  }
                  else
                  {
                     Parent.PointVms[0].OnPropertyChanged("PosX");
                     Parent.PointVms[0].OnPropertyChanged("AbsPosX");
                  }
               }

               CompoundObjectViewModel p = Parent.ParentVm;

               while (p != null)
               {
                  p.OnPropertyChanged("BoundingBox");
                  p = p.ParentVm;
               }
            }

            OnPropertyChanged("PosX");
            OnPropertyChanged("AbsPosX");
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

            if (_modelObject == null) return parentPos;

            return /*parentPos + */_modelObject.PosX;
         }
      }


      public double PosY
      {
         get
         {
            if (ModelObject == null) return 0;

            return ModelObject.PosY;
         }
         set
         {
            if (ModelObject == null) return;

            ModelObject.PosY = value;

            if (Parent != null)
            {
               Parent.OnPropertyChanged("Points");
               Parent.OnPropertyChanged("AbsPoints");
               Parent.OnPropertyChanged("BoundingBox");

               // We must also make sure that the previous point in the collection
               // is updated since the line between them must be redrawn
               int i = Parent.PointVms.IndexOf(this);

               if (i != -1)
               {
                  if (i < Parent.PointVms.Count() - 1)
                  {
                     Parent.PointVms[i + 1].OnPropertyChanged("PosY");
                     Parent.PointVms[i + 1].OnPropertyChanged("AbsPosY");
                  }
                  else
                  {
                     Parent.PointVms[0].OnPropertyChanged("PosY");
                     Parent.PointVms[0].OnPropertyChanged("AbsPosY");
                  }
               }

               CompoundObjectViewModel p = Parent.ParentVm;

               while (p != null)
               {
                  p.OnPropertyChanged("BoundingBox");
                  p = p.ParentVm;
               }
            }

            OnPropertyChanged("PosY");
            OnPropertyChanged("AbsPosY");
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

            if (_modelObject == null) return parentPos;

            return /* parentPos + */ _modelObject.PosY;
         }
      }


      public uint Id
      {
         get
         {
            if (ModelObject == null) return 0;

            return ModelObject.Id;
         }
         set
         {
            if (ModelObject == null) return;

            ModelObject.Id = value;
            OnPropertyChanged("Id");
         }
      }

      //public bool IsSelected
      //{
      //   get { return _isSelected; }
      //   set
      //   {
      //      _isSelected = value;
      //      OnPropertyChanged("IsSelected");
      //   }
      //}

      #endregion
   }
}
