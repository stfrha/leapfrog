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

      // Sets both Width and Height from the texture's pixel size divided by
      // GlobalConstants.TexturePixelsPerUnit.
      void SetSizeFromTexture();
   }
}
