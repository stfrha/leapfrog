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
   public class LfStaticPolygonViewModel : LfScalableTexturePolygonViewModel
   {
      #region Declarations

      private BorderTextureViewModel _groundBorder = null;
      private BorderTextureViewModel _ceilingBorder = null;
      private BorderTextureViewModel _leftWallBorder = null;
      private BorderTextureViewModel _rightWallBorder = null;

      #endregion

      #region Constructors

      public LfStaticPolygonViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm, 
         LfStaticPolygon modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;

         GroundBorder = new BorderTextureViewModel(this, modelObject.PolygonBorder.GroundBorder);
         CeilingBorder = new BorderTextureViewModel(this, modelObject.PolygonBorder.CeilingBorder);
         LeftWallBorder = new BorderTextureViewModel(this, modelObject.PolygonBorder.LeftWallBorder);
         RightWallBorder = new BorderTextureViewModel(this, modelObject.PolygonBorder.RightWallBorder);
      }

      #endregion

      #region Properties

      public new LfStaticPolygon LocalModelObject
      {
         get { return (LfStaticPolygon)ModelObject; }
      }

      public double LeftGroundAngle
      {
         get { return ((LfStaticPolygon)ModelObject).PolygonBorder.LeftGroundAngle; }
         set
         {
            ((LfStaticPolygon)ModelObject).PolygonBorder.LeftGroundAngle = value;
            OnPropertyChanged("LeftGroundAngle");
            InvalidateAllVertices();
         }
      }

      public double RightGroundAngle
      {
         get { return ((LfStaticPolygon)ModelObject).PolygonBorder.RightGroundAngle; }
         set
         {
            ((LfStaticPolygon)ModelObject).PolygonBorder.RightGroundAngle = value;
            OnPropertyChanged("RightGroundAngle");
            InvalidateAllVertices();
         }
      }

      public double LeftCeilingAngle
      {
         get { return ((LfStaticPolygon)ModelObject).PolygonBorder.LeftCeilingAngle; }
         set
         {
            ((LfStaticPolygon)ModelObject).PolygonBorder.LeftCeilingAngle = value;
            OnPropertyChanged("LeftCeilingAngle");
            InvalidateAllVertices();
         }
      }

      public double RightCeilingAngle
      {
         get { return ((LfStaticPolygon)ModelObject).PolygonBorder.RightCeilingAngle; }
         set
         {
            ((LfStaticPolygon)ModelObject).PolygonBorder.RightCeilingAngle = value;
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

      #endregion

      #region public methods

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


      #endregion
   }
}
