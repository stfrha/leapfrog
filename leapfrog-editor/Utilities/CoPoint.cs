using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace LeapfrogEditor
{
   class CoPoint
   {
      #region Declarations

      private Point _p = new Point();
      private CompoundObjectViewModel _co;

      #endregion

      #region Constructor

      public CoPoint(Point p, CompoundObjectViewModel co)
      {
         _p = p;
         _co = co;
      }

      #endregion

      #region Properties

      public Point P
      {
         get { return _p; }
         set { _p = value; }
      }

      public ScenePoint SceneP
      {
         get
         {
            return GetScenePoint();
         }
         set
         {
            SetScenePoint(value);
         }
      }

      #endregion

      #region Private methods

      private ScenePoint GetScenePoint()
      {
         ScenePoint sp = new ScenePoint(_p);
         sp.P.Offset(_co.PosX, _co.PosY);
         return sp;
      }

      private void SetScenePoint(ScenePoint sp)
      {
         _p = sp.P;
         _p.Offset(-_co.PosX, -_co.PosY);
      }

      #endregion

   }
}
