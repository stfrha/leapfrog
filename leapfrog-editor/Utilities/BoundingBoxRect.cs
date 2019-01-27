using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace LeapfrogEditor
{
   class BoundingBoxRect
   {
      private double l;
      private double r;
      private double t;
      private double b;
      
      public BoundingBoxRect()
      {
         l = double.MaxValue;
         r = double.MinValue;
         t = double.MaxValue;
         b = double.MinValue;
      }

      public Rect BoundingBox
      {
         get
         {
            return new Rect(new Point(l, t), new Point(r, b));
         }
         set { }
      }

      public void AddRect(Rect box)
      {
         if (box.X < l)
         {
            l = box.X;
         }

         if (box.X + box.Width > r)
         {
            r = box.X + box.Width;
         }

         if (box.Y < t)
         {
            t = box.Y;
         }

         if (box.Y + box.Height > b)
         {
            b = box.Y + box.Height;
         }
      }
   }
}
