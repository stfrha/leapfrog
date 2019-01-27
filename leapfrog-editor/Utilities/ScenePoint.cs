using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace LeapfrogEditor
{
   class ScenePoint
   {
      #region Declarations

      private Point _p = new Point();

      #endregion

      #region Constructor

      public ScenePoint(Point p)
      {
         _p = p;
      }

      #endregion

      #region Properties

      public Point P
      {
         get { return _p; }
         set { _p = value; }
      }

      #endregion

   }
}
