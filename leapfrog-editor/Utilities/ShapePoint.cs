using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace LeapfrogEditor
{
   class ShapePoint
   {
      #region Declarations

      // This is in local coordinates 
      private Point _p = new Point();
      private LfShapeViewModel _shape;

      #endregion

      #region Constructor

      public ShapePoint(Point p, LfShapeViewModel shape)
      {
         _p = p;
         _shape = shape;
      }

      #endregion

      #region Properties

      public Point P
      {
         get { return _p; }
         set { _p = value; }
      }

      public Point RotatedPoint
      {
         get
         {
            Point p2 = new Point();

            p2.X = _p.X * Math.Cos(_shape.Angle) - Math.Sin(_shape.Angle) * -_p.Y;
            p2.Y = -(_p.X * Math.Sin(_shape.Angle) + Math.Cos(_shape.Angle) * -_p.Y);

            return p2;            
         }
      }

      public CoPoint CoP
      {
         get
         {
            return GetCoPoint();
         }
         set
         {
            SetCoPoint(value);
         }
      }

      public ScenePoint ScenP
      {
         get
         {
            CoPoint cop = GetCoPoint();

            return cop.ScenP;
         }
         set
         {
            CoPoint cop = new CoPoint(_shape.Parent);

            cop.ScenP = value;

            SetCoPoint(cop);
         }
      }

      #endregion

      #region Private methods

      private CoPoint GetCoPoint()
      {
         // Transform Shape point to CoPoint using PosX, PosY and angle
         Point p = RotatedPoint;

         p.Offset(_shape.PosX, _shape.PosY);

         CoPoint cop = new CoPoint(p, _shape.Parent);

         return cop;
      }

      private void SetCoPoint(CoPoint sp)
      {
         _p = sp.P;

         // Transform CoPoint to ShapePoint using PosX, PosY and angle

         _p.Offset(-_shape.PosX, -_shape.PosY);
      }

      #endregion

   }
}
