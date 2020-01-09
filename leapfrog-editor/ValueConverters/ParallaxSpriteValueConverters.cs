using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;

namespace LeapfrogEditor
{
   class ParallaxSpriteScaleConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         // Values should be: 
         // PosX of sprite in stage coordinates
         // PosY of sprite in stage coordinates
         // PosX of view port in stage coordinates
         // PosY of view port in stage coordinates
         // Content scale
         // Width of view port
         // Height of view port
         // Stage width
         // Stage Height
         // Parallax amount
         // Returns scale x or y value, depending on parameter.

         if (values.Count() < 10)
         {
            return (double)1.0;
         }

         double absPosX = (double)values[0];
         double absPosY = (double)values[1];
         double vpPosX = (double)values[2] - 2000;
         double vpPosY = (double)values[3] - 2000;
         double contentScale = (double)values[4];
         double vpWidth = (double)values[5];
         double vpHeight = (double)values[6];
         double vpWidthSc = vpWidth / contentScale;
         double vpHeightSc = vpHeight / contentScale;
         double stageWidth = (double)values[7];
         double stageHeight = (double)values[8];
         double parallaxAmount = (double)values[9];

         string message = "PosX: " + absPosX.ToString("0.0") + ", PosY: " + absPosY.ToString("0.0") + ", ";
         message += "vpPosX: " + vpPosX.ToString("0.0") + ", vpPosY: " + vpPosY.ToString("0.0") + ", ";
         message += "scale: " + contentScale.ToString("0.0") + ", ";
         message += "vpWidth: " + vpWidth.ToString("0.0") + ", vpHeight: " + vpHeight.ToString("0.0") + ", ";
         message += "stageWidth: " + stageWidth.ToString("0.0") + ", stageHeight: " + stageHeight.ToString("0.0") + ", ";
         message += "pA: " + parallaxAmount.ToString("0.0");

         System.Diagnostics.Debug.WriteLine(message);

         // The scale should go from 1 (when parallax amount is 0) to vpWidthSc / stageWidth (when pA is 1) 
         // but only if (vpWidthSc < stageWidth) or (vpHeightSc < stageHeight)


         if (parameter as string == "x")
         {
            if (vpWidthSc < stageWidth)
            {
               return (double)(1 + (vpWidthSc / stageWidth - 1) * parallaxAmount);
            }

            return (double)1.0;
         }

         if (vpHeightSc < stageHeight)
         {
            return (double)(1 + (vpHeightSc / stageHeight - 1) * parallaxAmount);
         }

         return (double)1.0;
      }

      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         throw new NotImplementedException();
      }
   }

   class ParallaxSpriteTranslateConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         // Values should be: 
         // PosX of sprite in stage coordinates
         // PosY of sprite in stage coordinates
         // PosX of view port in stage coordinates
         // PosY of view port in stage coordinates
         // Content scale
         // Width of view port
         // Height of view port
         // Stage width
         // Stage Height
         // Parallax amount
         // Returns scale x or y value, depending on parameter.

         if (values.Count() < 10)
         {
            return (double)1.0;
         }

         double absPosX = (double)values[0];
         double absPosY = (double)values[1];
         double vpPosX = (double)values[2] - 2000;
         double vpPosY = (double)values[3] - 2000;
         double contentScale = (double)values[4];
         double vpWidth = (double)values[5];
         double vpHeight = (double)values[6];
         double vpWidthSc = vpWidth / contentScale;
         double vpHeightSc = vpHeight / contentScale;
         double stageWidth = (double)values[7];
         double stageHeight = (double)values[8];
         double parallaxAmount = (double)values[9];

         double vpCenterPosXSc = vpPosX + vpWidthSc / 2;
         double vpCenterPosYSc = vpPosY + vpHeightSc / 2;

         string message = "PosX: " + absPosX.ToString("0.0") + ", PosY: " + absPosY.ToString("0.0") + ", ";
         message += "vpPosX: " + vpPosX.ToString("0.0") + ", vpPosY: " + vpPosY.ToString("0.0") + ", ";
         message += "scale: " + contentScale.ToString("0.0") + ", ";
         message += "vpWidth: " + vpWidth.ToString("0.0") + ", vpHeight: " + vpHeight.ToString("0.0") + ", ";
         message += "stageWidth: " + stageWidth.ToString("0.0") + ", stageHeight: " + stageHeight.ToString("0.0") + ", ";
         message += "pA: " + parallaxAmount.ToString("0.0");

         System.Diagnostics.Debug.WriteLine(message);

         // The translation should go from 0 (when parallax amount is 0) to vpCenterPosXSc - stageWidth / 2 (when pA is 1) 
         // but only if (vpWidthSc < stageWidth) or (vpHeightSc < stageHeight)


         if (parameter as string == "x")
         {
            if (vpWidthSc < stageWidth)
            {
               return (double)((vpCenterPosXSc - stageWidth / 2) * parallaxAmount);
            }

            return (double)1.0;
         }

         if (vpHeightSc < stageHeight)
         {
            return (double)((vpCenterPosYSc - stageHeight / 2) * parallaxAmount);
         }

         return (double)1.0;
      }

      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         throw new NotImplementedException();
      }
   }
}
