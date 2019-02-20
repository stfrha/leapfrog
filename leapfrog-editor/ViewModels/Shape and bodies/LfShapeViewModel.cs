using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace LeapfrogEditor
{
   // Used to be ConditionalSelectTreeViewViewModel
   public class LfShapeViewModel : TreeViewViewModel, IPositionInterface
   {
      #region Declarations

      private LfShape _modelObject;

      #endregion

      #region Constructors

      public LfShapeViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = null;
      }

      #endregion

      #region Properties

      public LfShape ModelObject
      {
         get
         {
            if (_modelObject == null) return null;
            return _modelObject;
         }
         set
         {
            _modelObject = value;
            OnPropertyChanged("");
         }
      }

      public virtual string Name
      {
         get
         {
            if (_modelObject == null) return "";

            return _modelObject.Name;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.Name = value;
            OnPropertyChanged("Name");
         }
      }

      public string Texture
      {
         get
         {
            if (_modelObject == null) return "";

            return _modelObject.Texture;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.Texture = value;
            OnPropertyChanged("Texture");
         }
      }

      public virtual double PosX
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.PosX;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.PosX = value;
            OnPropertyChanged("PosX");
            OnPropertyChanged("BoundingBox");

            CompoundObjectViewModel p = ParentVm;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p.InvalidateJoints();
               p = p.ParentVm;
            }
         }
      }


      public virtual double PosY
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.PosY;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.PosY = value;
            OnPropertyChanged("PosY");
            OnPropertyChanged("BoundingBox");

            CompoundObjectViewModel p = ParentVm;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p.InvalidateJoints();
               p = p.ParentVm;
            }
         }
      }

      public double Angle
      {
         get
         {
            if (_modelObject == null) return 0;

            // Angle is defined differently (invers) for the editor and the game,
            // In an attempt to make as few changes as possible, I do the inversion
            // in the getter and setter of this property.
            return - _modelObject.Angle;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.Angle = -value;
            OnPropertyChanged("Angle");
            OnPropertyChanged("BoundingBox");
            ParentVm.UpdateAnglePropertyOfSystems();

            CompoundObjectViewModel p = ParentVm;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p.InvalidateJoints();
               p = p.ParentVm;
            }
            InvalidateAll();
         }
      }

      public uint ZLevel
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.ZLevel;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.ZLevel = value;
            OnPropertyChanged("ZLevel");
         }
      }

      public string CollisionEntity
      {
         get
         {
            if (_modelObject == null) return "";

            return _modelObject.CollisionEntity;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.CollisionEntity = value;
            OnPropertyChanged("CollisionEntity");
         }
      }

      public uint CollisionCategory
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.CollisionCategory;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.CollisionCategory = value;
            OnPropertyChanged("CollisionCategory");
         }
      }

      public uint CollisionMask
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.CollisionMask;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.CollisionMask = value;
            OnPropertyChanged("CollisionMask");
         }
      }

      public Rect BoundingBox
      {
         get
         {
            return GetBoundingBox();
            //Rect r = new Rect(0, 0, Width, Height);
            //r.Offset(new Vector(-AnchorX * Width, -AnchorY * Height));
            //return r;
         }
         set
         { }
      }

      //public new bool IsSelected
      //{
      //   get { return _isSelected; }
      //   set
      //   {
      //      if (MainVm.AmISelectable(this))
      //      {
      //         _isSelected = value;

      //         if (!_isSelected)
      //         {
      //            DeselectAllChildren();
      //         }

      //         OnPropertyChanged("IsSelected");
      //      }
      //   }
      //}

      #endregion

      #region protected Methods

      protected virtual void DeselectAllChildren()
      {
         // Only polygon shapes has children (so far), se we do nothing here
      }

      protected virtual Rect GetBoundingBox()
      {
         // Base class implementation returns default rect
         return new Rect(new Size(200, 200));
      }

      #endregion

      #region public Methods

      public void RotateShape(int delta, bool fine)
      {
         double increment;

         if (fine)
         {
            increment = (double)delta * 0.1;
         }
         else
         {
            increment = (double)delta * 1;
         }

         if (IsSelected)
         {
            Angle += increment / 120 / 180 * Math.PI;
            //InvalidateAll();
         }
      }


      public virtual void InvalidateAll()
      {
         OnPropertyChanged("");
      }

      //// Let's try to define some points here:
      //// A shape point is a point expressed in the coordinate system
      //// of the shape.
      //// There are two types of shape points: local points and rotated points.
      //// The local point is the point of the shape without it being rotated.
      //// The rotated is the local points rotated to the Angle property.

      //// A rotated point is expressed in the shape's coordinate system
      //// i.e. using the shape PosX,PosY as origo but it is rotated
      //// according to the Angle property
      //public Point RotatedPointFromLocal(Point localPoint)
      //{
      //   Point p2 = new Point();

      //   double cosa = Math.Cos(Angle);
      //   double sina = Math.Sin(Angle);

      //   p2.X = localPoint.X * cosa - sina * -localPoint.Y;
      //   p2.Y = -(localPoint.X * sina + cosa * -localPoint.Y);

      //   return p2;
      //}

      //// A local point is expressed in the shape's coordinate system
      //// i.e. using the shape PosX,PosY as origo. It is the coordinate
      //// if the shape has no rotation (As if the Angle property is zero)
      //public Point LocalPointFromRotated(Point localPoint)
      //{
      //   Point p2 = new Point();

      //   double cosa = Math.Cos(-Angle);
      //   double sina = Math.Sin(-Angle);

      //   p2.X = localPoint.X * cosa - sina * -localPoint.Y;
      //   p2.Y = -(localPoint.X * sina + cosa * -localPoint.Y);


      //   return p2;
      //}

      // This method returns with the supplied point (expressed in the 
      // shape's coordinate system) converted to the coordinate system
      // of the scene (i.e. that of the top level CompoundObject).
      public Point ScenePointFromShape(Point shapePoint)
      {
         return ParentVm.GetScenePointFromCoPoint(ParentVm.ShapePointInCo(shapePoint, this));
      }

      public Point ShapePointFromScene(Point scenePoint)
      {
         Point coPoint = ParentVm.GetCoPointFromScenePoint(scenePoint);

         return ParentVm.CoPointInShape(coPoint, this);
      }

      #endregion
   }
}
