using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LeapfrogEditor
{
   public static class GlobalConstants
   {
      public static string DataDirectory = @"..\..\..\leapfrog-game\data\";
      public static string ImageDirectory = DataDirectory +  @"images\";

      // Texture resolution used by the Blender sprite pipeline:
      // one game unit (= one Blender unit) is rendered as this many pixels.
      public static double TexturePixelsPerUnit = 20.0;
   }
}
