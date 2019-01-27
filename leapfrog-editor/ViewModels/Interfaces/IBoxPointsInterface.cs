using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LeapfrogEditor
{
   interface IBoxPointsInterface
   {
      ObservableCollection<LfPointViewModel> PointVms
      {
         get;
         set;
      }

   }
}
