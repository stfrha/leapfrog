using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace LeapfrogEditor
{
   class TextureBorderHelperClass
   {
      public static Point GetOriginalPoint(LfDragablePointViewModel myPoint)
      {
         Point origP = new Point(myPoint.PosX, myPoint.PosY);

         return origP;
      }

      public static Point GetPreviousPoint(LfDragablePointViewModel myPoint)
      {
         LfStaticPolygonViewModel polygon = (LfStaticPolygonViewModel)myPoint.Parent;

         int i = polygon.PointVms.IndexOf(myPoint);

         if (i == -1)
         {
            return new Point(0, 0);
         }

         LfDragablePointViewModel previousVertex;

         if (i > 0)
         {
            previousVertex = polygon.PointVms[i - 1];
         }
         else
         {
            previousVertex = polygon.PointVms[polygon.PointVms.Count() - 1];
         }

         Point prevP = new Point(previousVertex.PosX, previousVertex.PosY);

         return prevP;
      }

      public static double GetAngle(LfDragablePointViewModel myPoint, Point origP, Point prevP)
      {
         LfStaticPolygonViewModel polygon = (LfStaticPolygonViewModel)myPoint.Parent;

         Vector diffV = origP - prevP;

         double angle = Math.Atan2(diffV.Y, diffV.X);

         angle += Math.PI;

         if (angle > Math.PI)
         {
            angle -= Math.PI * 2;
         }

         return angle / Math.PI * 180;
      }

      public static Vector GetOffset(LfDragablePointViewModel myPoint, Point origP, Point prevP, double angle)
      {
         Vector diffV = origP - prevP;
         double distance = diffV.Length;
         Vector rotatedNormal = new Vector(-Math.Sin(angle * Math.PI / 180), Math.Cos(angle * Math.PI / 180));
         BorderTextureViewModel border = TextureBorderHelperClass.GetTextureViewModel(myPoint, angle);
         Vector offset = -rotatedNormal * border.HorisontalOffset;

         return offset;
      }


      public static BorderTextureViewModel GetTextureViewModel(LfDragablePointViewModel myPoint, double angle)
      {
         // Select view model accordingly to angle
         LfStaticPolygonViewModel polygon = (LfStaticPolygonViewModel)myPoint.Parent;

         double degAngle = angle;

         if ((degAngle <= polygon.RightGroundAngle) && (degAngle > polygon.LeftGroundAngle))
         {
            return polygon.GroundBorder;
         }

         if ((degAngle > polygon.RightGroundAngle) && (degAngle < polygon.RightCeilingAngle))
         {
            return polygon.RightWallBorder;
         }


         if ((degAngle < polygon.LeftGroundAngle) && (degAngle > polygon.LeftCeilingAngle))
         {
            return polygon.LeftWallBorder;
         }

         return polygon.CeilingBorder;

      }
   }

   class TextureBorderVisibilityMultiConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 3)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] is LfDragablePointViewModel))
            {
               LfDragablePointViewModel origVertex = (LfDragablePointViewModel)values[2];

               Point currP = TextureBorderHelperClass.GetOriginalPoint(origVertex);
               Point prevP = TextureBorderHelperClass.GetPreviousPoint(origVertex);
               double angle = TextureBorderHelperClass.GetAngle(origVertex, currP, prevP);
               BorderTextureViewModel border = TextureBorderHelperClass.GetTextureViewModel(origVertex, angle);

               if ((border.Texture == "") || (border.Texture == "notDefined"))
               {
                  return Visibility.Collapsed;
               }

               return Visibility.Visible;
            }
         }

         return null;
      }

      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         throw new NotImplementedException();
      }
   }

   class TextureBorderWidthMultiConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 3)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] is LfDragablePointViewModel))
            {
               LfDragablePointViewModel origVertex = (LfDragablePointViewModel)values[2];

               Point currP = TextureBorderHelperClass.GetOriginalPoint(origVertex);
               Point prevP = TextureBorderHelperClass.GetPreviousPoint(origVertex);
               Vector v = (currP - prevP);
               return v.Length;

            }
         }

         return null;
      }

      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         throw new NotImplementedException();
      }
   }

   class TextureBorderHeightMultiConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 3)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] is LfDragablePointViewModel))
            {
               LfDragablePointViewModel origVertex = (LfDragablePointViewModel)values[2];

               Point currP = TextureBorderHelperClass.GetOriginalPoint(origVertex);
               Point prevP = TextureBorderHelperClass.GetPreviousPoint(origVertex);
               double angle = TextureBorderHelperClass.GetAngle(origVertex, currP, prevP);
               BorderTextureViewModel border = TextureBorderHelperClass.GetTextureViewModel(origVertex, angle);

               return border.TextureHeight;
            }
         }

         return null;
      }

      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         throw new NotImplementedException();
      }
   }

   class TextureBorderOffsetMultiConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 3)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] is LfDragablePointViewModel))
            {
               LfDragablePointViewModel origVertex = (LfDragablePointViewModel)values[2];

               Point currP = TextureBorderHelperClass.GetOriginalPoint(origVertex);
               Point prevP = TextureBorderHelperClass.GetPreviousPoint(origVertex);
               double angle = TextureBorderHelperClass.GetAngle(origVertex, currP, prevP);
               BorderTextureViewModel border = TextureBorderHelperClass.GetTextureViewModel(origVertex, angle);
               Vector offset = TextureBorderHelperClass.GetOffset(origVertex, currP, prevP, angle);

               if (parameter as string == "x")
               {
                  return offset.X;
               }
               else
               {
                  return offset.Y;
               }
            }
         }

         return null;
      }

      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         throw new NotImplementedException();
      }
   }

   class TextureBorderAngleMultiConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 4)
         {
            if ((values[0] is double) && (values[1] is double) &&
               (values[2] is double) && (values[3] is LfDragablePointViewModel))
            {
               LfDragablePointViewModel origVertex = (LfDragablePointViewModel)values[3];
               double shapeAngle = (double)values[2];

               Point currP = TextureBorderHelperClass.GetOriginalPoint(origVertex);
               Point prevP = TextureBorderHelperClass.GetPreviousPoint(origVertex);
               double angle = TextureBorderHelperClass.GetAngle(origVertex, currP, prevP);
               return angle + shapeAngle;
            }
         }

         return null;
      }

      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         throw new NotImplementedException();
      }
   }

   class TextureBorderRectMultiValueConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 3)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] is LfDragablePointViewModel))
            {
               LfDragablePointViewModel origVertex = (LfDragablePointViewModel)values[2];

               Point currP = TextureBorderHelperClass.GetOriginalPoint(origVertex);
               Point prevP = TextureBorderHelperClass.GetPreviousPoint(origVertex);
               double angle = TextureBorderHelperClass.GetAngle(origVertex, currP, prevP);
               BorderTextureViewModel border = TextureBorderHelperClass.GetTextureViewModel(origVertex, angle);

               Rect r = new Rect(0, 0, border.TextureWidth, border.TextureHeight);

               return r;
            }
         }

         return null;
      }
      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         throw new NotImplementedException();
      }
   }

   class TextureBorderTexturePathMultiConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 3)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] is LfDragablePointViewModel))
            {
               LfDragablePointViewModel origVertex = (LfDragablePointViewModel)values[2];

               Point currP = TextureBorderHelperClass.GetOriginalPoint(origVertex);
               Point prevP = TextureBorderHelperClass.GetPreviousPoint(origVertex);
               double angle = TextureBorderHelperClass.GetAngle(origVertex, currP, prevP);
               BorderTextureViewModel border = TextureBorderHelperClass.GetTextureViewModel(origVertex, angle);

               string texture = border.Texture;

               if ((texture == "") || (texture == "notDefined"))
               {
                  return null;
               }

               string s = GlobalConstants.ImageDirectory + texture + ".png";
               string fullPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
               string fullFileName = System.IO.Path.Combine(fullPath, s);

               return new BitmapImage(new Uri(fullFileName));
            }
         }

         return null;
      }

      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         throw new NotImplementedException();
      }
   }

}
