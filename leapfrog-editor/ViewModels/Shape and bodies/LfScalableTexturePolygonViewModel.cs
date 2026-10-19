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
   public class LfScalableTexturePolygonViewModel : LfPolygonViewModel
   {
      #region Declarations

      private BorderTextureViewModel _groundBorder = null;
      private BorderTextureViewModel _ceilingBorder = null;
      private BorderTextureViewModel _leftWallBorder = null;
      private BorderTextureViewModel _rightWallBorder = null;

      #endregion

      #region Constructors

      public LfScalableTexturePolygonViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
      }

      #endregion

      #region Properties

      public new LfScalableTexturePolygon LocalModelObject
      {
         get { return (LfScalableTexturePolygon)ModelObject; }
      }

      // --- Polygon border (shared by static and sprite polygons) ---------

      public double LeftGroundAngle
      {
         get { return LocalModelObject.PolygonBorder.LeftGroundAngle; }
         set
         {
            LocalModelObject.PolygonBorder.LeftGroundAngle = value;
            OnPropertyChanged("LeftGroundAngle");
            InvalidateAllVertices();
         }
      }

      public double RightGroundAngle
      {
         get { return LocalModelObject.PolygonBorder.RightGroundAngle; }
         set
         {
            LocalModelObject.PolygonBorder.RightGroundAngle = value;
            OnPropertyChanged("RightGroundAngle");
            InvalidateAllVertices();
         }
      }

      public double LeftCeilingAngle
      {
         get { return LocalModelObject.PolygonBorder.LeftCeilingAngle; }
         set
         {
            LocalModelObject.PolygonBorder.LeftCeilingAngle = value;
            OnPropertyChanged("LeftCeilingAngle");
            InvalidateAllVertices();
         }
      }

      public double RightCeilingAngle
      {
         get { return LocalModelObject.PolygonBorder.RightCeilingAngle; }
         set
         {
            LocalModelObject.PolygonBorder.RightCeilingAngle = value;
            OnPropertyChanged("RightCeilingAngle");
            InvalidateAllVertices();
         }
      }

      public BorderTextureViewModel GroundBorder
      {
         get { return _groundBorder; }
         set
         {
            _groundBorder = value;
            OnPropertyChanged("GroundBorder");
            InvalidateAllVertices();
         }
      }

      public BorderTextureViewModel CeilingBorder
      {
         get { return _ceilingBorder; }
         set
         {
            _ceilingBorder = value;
            OnPropertyChanged("CeilingBorder");
            InvalidateAllVertices();
         }
      }

      public BorderTextureViewModel LeftWallBorder
      {
         get { return _leftWallBorder; }
         set
         {
            _leftWallBorder = value;
            OnPropertyChanged("LeftWallBorder");
            InvalidateAllVertices();
         }
      }

      public BorderTextureViewModel RightWallBorder
      {
         get { return _rightWallBorder; }
         set
         {
            _rightWallBorder = value;
            OnPropertyChanged("RightWallBorder");
            InvalidateAllVertices();
         }
      }



      public double TextureWidth
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.TextureWidth;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.TextureWidth = value;
            OnPropertyChanged("TextureWidth");
         }
      }

      public double TextureHeight
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.TextureHeight;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.TextureHeight = value;
            OnPropertyChanged("TextureHeight");
         }
      }

      public double TextureOffsetX
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.TextureOffsetX;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.TextureOffsetX = value;
            OnPropertyChanged("TextureOffsetX");
         }
      }

      public double TextureOffsetY
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.TextureOffsetY;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.TextureOffsetY = value;
            OnPropertyChanged("TextureOffsetY");
         }
      }


      #endregion

   
      // The border decorations are tinted like the polygon
      protected override void OnTintChanged()
      {
         base.OnTintChanged();
         InvalidateAllVertices();
      }

      // Called by derived view models once ModelObject is set
      protected void CreateBorderVms()
      {
         PolygonBorder pb = LocalModelObject.PolygonBorder;
         GroundBorder = new BorderTextureViewModel(this, pb.GroundBorder);
         CeilingBorder = new BorderTextureViewModel(this, pb.CeilingBorder);
         LeftWallBorder = new BorderTextureViewModel(this, pb.LeftWallBorder);
         RightWallBorder = new BorderTextureViewModel(this, pb.RightWallBorder);
      }

      public void InvalidateAllVertices()
      // If there is a change in any of the properties
      // that affects the border decorations, all the
      // vertices need to be updated. This is done here
      {
         foreach (LfDragablePointViewModel dpvm in PointVms)
         {
            dpvm.OnPropertyChanged("");
         }
         OnPropertyChanged("Points");
         OnPropertyChanged("AbsPoints");

      }
   }
}
