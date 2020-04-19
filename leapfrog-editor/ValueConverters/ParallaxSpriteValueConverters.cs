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

         double contentScale = (double)values[0];
         double vpWidth = (double)values[1];
         double vpHeight = (double)values[2];
         double vpWidthSc = vpWidth / contentScale;
         double vpHeightSc = vpHeight / contentScale;
         double stageWidthSc = (double)values[3];
         double stageHeight = (double)values[4];
         double parallaxAmount = (double)values[5];

         //string message = "PosX: " + absPosXSc.ToString("0.0") + ", PosY: " + absPosYSc.ToString("0.0") + ", ";
         //message += "vpPosX: " + vpPosX.ToString("0.0") + ", vpPosY: " + vpPosY.ToString("0.0") + ", ";
         //message += "scale: " + contentScale.ToString("0.0") + ", ";
         //message += "vpWidth: " + vpWidth.ToString("0.0") + ", vpHeight: " + vpHeight.ToString("0.0") + ", ";
         //message += "stageWidthSc: " + stageWidthSc.ToString("0.0") + ", stageHeight: " + stageHeight.ToString("0.0") + ", ";
         //message += "pA: " + parallaxAmount.ToString("0.0");
         //System.Diagnostics.Debug.WriteLine(message);

         // The scale should go from 1 (when parallax amount is 0) to vpWidthSc / stageWidthSc (when pA is 1) 
         // but only if (vpWidthSc < stageWidthSc) or (vpHeightSc < stageHeight)


         if (parameter as string == "x")
         {
            if (vpWidthSc < stageWidthSc)
            {
               return (double)(1 + (vpWidthSc / stageWidthSc - 1) * parallaxAmount);
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

         double absPosXSc = (double)values[0];
         double absPosYSc = (double)values[1];
         double vpPosXSc = (double)values[2] - 2000;
         double vpPosYSc = (double)values[3] - 2000;
         double contentScale = (double)values[4];
         double vpWidthVpc = (double)values[5];
         double vpHeightVpc = (double)values[6];
         double vpWidthSc = vpWidthVpc / contentScale;
         double vpHeightSc = vpHeightVpc / contentScale;
         double stageWidthSc = (double)values[7];
         double stageHeightSc = (double)values[8];
         double parallaxAmount = (double)values[9];

         double vpCenterPosXSc = vpPosXSc + vpWidthSc / 2;
         double vpCenterPosYSc = vpPosYSc + vpHeightSc / 2;

         string message = "PosX: " + absPosXSc.ToString("0.0") + " [SC], PosY: " + absPosYSc.ToString("0.0") + " [SC], ";
         message += "vpPosX: " + vpPosXSc.ToString("0.0") + " [SC], vpPosY: " + vpPosYSc.ToString("0.0") + " [SC], ";
         message += "scale: " + contentScale.ToString("0.0") + ", ";
         message += "vpWidth: " + vpWidthVpc.ToString("0.0") + " [Vpc], vpHeight: " + vpHeightVpc.ToString("0.0") + " [Vpc], ";
         message += "stageWidthSc: " + stageWidthSc.ToString("0.0") + " [SC], stageHeight: " + stageHeightSc.ToString("0.0") + " [SC], ";
         message += "pA: " + parallaxAmount.ToString("0.0");
         System.Diagnostics.Debug.WriteLine(message);

         // The translation should go from 0 (when parallax amount is 0) to (vpCenterPosXSc - stageWidthSc / 2) + absPosXSc / stageWidthSc + vpWidthSc (when pA is 1) 
         // but only if (vpWidthSc < stageWidthSc) or (vpHeightSc < stageHeight)

         // Translation should be in two steps
         // First the sprite is placed on a virtual, background sprite that is the sized of the stage (this should be absPosXSc and absPosYSc)
         // This corresponds to the static placement of the sprite onto the parallax background sprite
         // 
         // Then the background is scaled and positioned according to the game formula
         // The total translation is the sum of the background position plus the sprite position
         
         //

         if (parameter as string == "x")
         {
            if (vpWidthSc < stageWidthSc)
            {
               // The game formula uses two positions. wantedPosVp and panPos.
               // wantedPos is the position of the viewport (leftish, rightish, topish, bottomish or dead center)
               // where the panPos should be shown.
               // The panPos is the position of the object that should be placed at the wantedPos,
               // for instance the leapfrog platform.
               // To get any bearing on this, we need the view port to exists in the editor also.
               // Lets make it as a see-through window of an, otherwise gray, forground covering all.
               // Lets also make this possible to hide. 
               // The position of the VP is calculated as in the game, 
               

               // In the editor, assume that the center of the viewport is the 
               // panorate point (but in scene coordinates). And assume that the center of the viewport is the wantedPos.
               // In the game, there is a zoom-factor between stage and view port, m_stageToViewPortScale. In the editor 
               // there is a zoom-factor for the view, contentScale. These are not the same zoom-factors. At this time
               // lets set the m_stageToViewPortScale to 1.0. 
               double panPosXSc = vpCenterPosXSc;
               double panPosYSc = vpCenterPosYSc;
               double wantedPosXVp = vpWidthVpc / 2.0;
               double wantedPosYVp = vpHeightVpc / 2.0;

               // IN the game, there are restrictions of how far the panoration is allowed, so that
               // the user cannot see beyond the outer limits. We don't need that here.





               double wantedVpPosX = absPosXSc / stageWidthSc * vpWidthVpc;
               double wantedVpPosXSc = wantedVpPosX / contentScale;
               double try1 = ((vpCenterPosXSc + wantedVpPosXSc) * parallaxAmount);
               double try2 = ((vpCenterPosXSc - stageWidthSc / 2 + absPosXSc / stageWidthSc * vpWidthSc) * parallaxAmount);
               double try3 = ((vpCenterPosXSc - stageWidthSc / 2) * parallaxAmount);

               return (double)try3;
            }

            return (double)1.0;
         }

         if (vpHeightSc < stageHeightSc)
         {
            double wantedVpPosY = absPosYSc / stageHeightSc * vpHeightVpc;
            double wantedVpPosYSc = wantedVpPosY / contentScale;
            double try1 = ((vpCenterPosYSc + wantedVpPosYSc) * parallaxAmount);
            double try2 = ((vpCenterPosYSc - stageHeightSc / 2 + absPosYSc / stageHeightSc * vpHeightSc) * parallaxAmount);
            double try3 = ((vpCenterPosYSc - stageHeightSc / 2) * parallaxAmount);

            return (double)try3;
         }

         return (double)1.0;
      }

      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         throw new NotImplementedException();
      }
   }
}
