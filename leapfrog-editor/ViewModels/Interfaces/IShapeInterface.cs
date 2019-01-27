using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

namespace LeapfrogEditor
{
   interface IShapeInterface : IPositionInterface
   {
      CompoundObjectViewModel Parent
      {
         get;
         set;
      }

      Rect BoundingBox
      {
         get;
         set;
      }

      bool IsSelected
      {
         get;
         set;
      }
   }
}
