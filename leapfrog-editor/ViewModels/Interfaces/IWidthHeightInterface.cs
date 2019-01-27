using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LeapfrogEditor
{
   interface IWidthHeightInterface
   {
      double Width
      {
         get;
         set;
      }

      double Height
      {
         get;
         set;
      }

      void SetHeightToTextureAspectRatio();

      void SetWidthToTextureAspectRatio();
   }
}
