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

      // --- Fade (see PolygonFade) ------------------------------------------

      private PolygonFade Fade
      {
         get
         {
            if (LocalModelObject.Fade == null)
            {
               LocalModelObject.Fade = new PolygonFade();
            }
            return LocalModelObject.Fade;
         }
      }

      // 0..1, 0 = no fade
      public double FadeAmount
      {
         get { return (LocalModelObject == null) ? 0 : Fade.Amount; }
         set
         {
            if (LocalModelObject == null) return;
            Fade.Amount = Math.Max(0.0, Math.Min(1.0, value));
            OnPropertyChanged("FadeAmount");
         }
      }

      // Direction the fade grows in, degrees: 0 = down, 90 = right, 180 = up, 270 = left
      public double FadeAngle
      {
         get { return (LocalModelObject == null) ? 0 : Fade.Angle; }
         set
         {
            if (LocalModelObject == null) return;
            Fade.Angle = value;
            OnPropertyChanged("FadeAngle");
         }
      }

      // Where the fade begins, 0..1 of the polygon's extent in its direction
      public double FadeStart
      {
         get { return (LocalModelObject == null) ? 0 : Fade.Start; }
         set
         {
            if (LocalModelObject == null) return;
            Fade.Start = value;
            OnPropertyChanged("FadeStart");
         }
      }

      // Where the fade is complete, 0..1
      public double FadeEnd
      {
         get { return (LocalModelObject == null) ? 1 : Fade.End; }
         set
         {
            if (LocalModelObject == null) return;
            Fade.End = value;
            OnPropertyChanged("FadeEnd");
         }
      }

      // rrggbb to fade towards (haze); empty = fade to transparent. An
      // invalid value is cleared, as for Color.
      public string FadeColor
      {
         get { return (LocalModelObject == null) ? "" : (Fade.Color ?? ""); }
         set
         {
            if (LocalModelObject == null) return;
            byte r, g, b, a;
            if (TextureTint.TryParse(value, 0, out r, out g, out b, out a))
            {
               string t = value.Trim().TrimStart('#').ToLowerInvariant();
               Fade.Color = t.Substring(0, 6);
            }
            else
            {
               Fade.Color = "";
            }
            OnPropertyChanged("FadeColor");
            OnPropertyChanged("FadeColorBrush");
         }
      }

      // The fade colour as a brush for the editor's haze layer (null when
      // the fade is to transparent)
      public Brush FadeColorBrush
      {
         get
         {
            byte r, g, b, a;
            if (!TextureTint.TryParse(FadeColor, 0, out r, out g, out b, out a))
            {
               return null;
            }
            SolidColorBrush brush = new SolidColorBrush(System.Windows.Media.Color.FromRgb(r, g, b));
            brush.Freeze();
            return brush;
         }
      }

      // --- Edge fade (see PolygonEdgeFade) ---------------------------------

      private PolygonEdgeFade EdgeFade
      {
         get
         {
            if (LocalModelObject.EdgeFade == null)
            {
               LocalModelObject.EdgeFade = new PolygonEdgeFade();
            }
            return LocalModelObject.EdgeFade;
         }
      }

      // How far in from the edge the fill fades (game units), 0 = off
      public double EdgeFadeWidth
      {
         get { return (LocalModelObject == null) ? 0 : EdgeFade.Width; }
         set
         {
            if (LocalModelObject == null) return;
            EdgeFade.Width = Math.Max(0.0, value);
            OnPropertyChanged("EdgeFadeWidth");
         }
      }

      // 0..1, how transparent the fill is at the edge
      public double EdgeFadeAmount
      {
         get { return (LocalModelObject == null) ? 1 : EdgeFade.Amount; }
         set
         {
            if (LocalModelObject == null) return;
            EdgeFade.Amount = Math.Max(0.0, Math.Min(1.0, value));
            OnPropertyChanged("EdgeFadeAmount");
         }
      }

      // The sides as saved: "all", or some of "top bottom left right"
      public string EdgeFadeSides
      {
         get { return (LocalModelObject == null) ? "all" : (EdgeFade.Sides ?? "all"); }
      }

      public bool EdgeFadeTop
      {
         get { return (LocalModelObject != null) && EdgeFade.HasSide("top"); }
         set { SetEdgeFadeSide("top", value); }
      }

      public bool EdgeFadeBottom
      {
         get { return (LocalModelObject != null) && EdgeFade.HasSide("bottom"); }
         set { SetEdgeFadeSide("bottom", value); }
      }

      public bool EdgeFadeLeft
      {
         get { return (LocalModelObject != null) && EdgeFade.HasSide("left"); }
         set { SetEdgeFadeSide("left", value); }
      }

      public bool EdgeFadeRight
      {
         get { return (LocalModelObject != null) && EdgeFade.HasSide("right"); }
         set { SetEdgeFadeSide("right", value); }
      }

      private void SetEdgeFadeSide(string side, bool on)
      {
         if (LocalModelObject == null) return;

         string[] all = { "top", "bottom", "left", "right" };
         List<string> sides = new List<string>();
         foreach (string sd in all)
         {
            bool has = (sd == side) ? on : EdgeFade.HasSide(sd);
            if (has)
            {
               sides.Add(sd);
            }
         }
         EdgeFade.Sides = (sides.Count == all.Length) ? "all" : string.Join(" ", sides);

         OnPropertyChanged("EdgeFadeSides");
         OnPropertyChanged("EdgeFadeTop");
         OnPropertyChanged("EdgeFadeBottom");
         OnPropertyChanged("EdgeFadeLeft");
         OnPropertyChanged("EdgeFadeRight");
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
