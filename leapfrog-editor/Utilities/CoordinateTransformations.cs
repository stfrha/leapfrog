using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace LeapfrogEditor
{
    class CoordinateTransformations
    {
      // Let's try to define some points here:
      // A shape point is a point expressed in the coordinate system
      // of the shape.
      // There are two types of shape points: local points and rotated points.
      // The local point is the point of the shape without it being rotated.
      // The rotated is the local points rotated to the Angle property.

      // A rotated point is expressed in the shape's coordinate system
      // i.e. using the shape PosX,PosY as origo but it is rotated
      // according to the Angle property
      static public Point RotatedPointFromLocal(Point localPoint, double angle)
      {
         Point p2 = new Point();

         double cosa = Math.Cos(angle);
         double sina = Math.Sin(angle);

         p2.X = localPoint.X * cosa - sina * -localPoint.Y;
         p2.Y = -(localPoint.X * sina + cosa * -localPoint.Y);

         return p2;
      }

      // A local point is expressed in the shape's coordinate system
      // i.e. using the shape PosX,PosY as origo. It is the coordinate
      // if the shape has no rotation (As if the Angle property is zero)
      static public Point LocalPointFromRotated(Point localPoint, double angle)
      {
         Point p2 = new Point();

         double cosa = Math.Cos(-angle);
         double sina = Math.Sin(-angle);

         p2.X = localPoint.X * cosa - sina * -localPoint.Y;
         p2.Y = -(localPoint.X * sina + cosa * -localPoint.Y);


         return p2;
      }


   }
}
