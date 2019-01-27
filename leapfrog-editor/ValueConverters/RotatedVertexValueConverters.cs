using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;

namespace LeapfrogEditor
{
   class MultiRotatedJointValueConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 3)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] is LfShapeViewModel))
            {
               Point pos = new Point((double)values[0], (double)values[1]);
               LfShapeViewModel shape = (LfShapeViewModel)values[2];
               Point rp = shape.RotatedPointFromLocal(pos);
               rp.Offset(shape.PosX, shape.PosY);

               if (parameter as string == "x")
               {
                  return rp.X;
               }
               else
               {
                  return rp.Y;
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


   class PreviousMultiRotatedTriangleVertexValueConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 3)
         {
            if ((values[0] is double) && (values[1] is LfDragablePointViewModel) && (values[2] is TriangleViewModel))
            {
               LfDragablePointViewModel origVertex = (LfDragablePointViewModel)values[1];
               LfPolygonViewModel polygon = origVertex.Parent;
               TriangleViewModel tvm = (TriangleViewModel)values[2];

               int i = tvm.PointVms.IndexOf(origVertex);

               if (i == -1)
               {
                  return null;
               }

               LfDragablePointViewModel vertex;

               if (i > 0)
               {
                  vertex = tvm.PointVms[i - 1];
               }
               else
               {
                  vertex = tvm.PointVms[tvm.PointVms.Count() - 1];
               }

               Point p = new Point(vertex.PosX, vertex.PosY);
               Point rp = polygon.RotatedPointFromLocal(p);

               if (parameter as string == "x")
               {
                  return rp.X;
               }
               else
               {
                  return rp.Y;
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

   class PreviousMultiRotatedVertexValueConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 2)
         {
            if ((values[0] is double) && (values[1] is LfDragablePointViewModel))
            {
               LfDragablePointViewModel origVertex = (LfDragablePointViewModel)values[1];
               LfPolygonViewModel polygon = origVertex.Parent;

               int i = polygon.PointVms.IndexOf(origVertex);

               if (i == -1)
               {
                  return null;
               }

               LfDragablePointViewModel vertex;

               if (i > 0)
               {
                  vertex = polygon.PointVms[i - 1];
               }
               else
               {
                  vertex = polygon.PointVms[polygon.PointVms.Count() - 1];
               }

               Point p = new Point(vertex.PosX, vertex.PosY);
               Point rp = polygon.RotatedPointFromLocal(p);

               if (parameter as string == "x")
               {
                  return rp.X;
               }
               else
               {
                  return rp.Y;
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

   class MultiRotatedVertexValueConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 2)
         {
            if ((values[0] is double) && (values[1] is LfDragablePointViewModel))
            {
               double pos = (double)values[0];
               LfDragablePointViewModel vertex = (LfDragablePointViewModel)values[1];
               LfShapeViewModel shape = vertex.Parent;

               Point p;

               if (parameter as string == "x")
               {
                  p = new Point(pos, vertex.PosY);
                  Point rp = shape.RotatedPointFromLocal(p);
                  return rp.X;
               }
               else
               {
                  p = new Point(vertex.PosX, pos);
                  Point rp = shape.RotatedPointFromLocal(p);
                  return rp.Y;
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

   class PreviousMultiRotatedPointValueConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 2)
         {
            if ((values[0] is double) && (values[1] is LfPointViewModel))
            {
               LfPointViewModel origVertex = (LfPointViewModel)values[1];
               IBoxPointsInterface boxVm = origVertex.PointsParent;
               LfShapeViewModel shapeVm = (LfShapeViewModel)origVertex.PointsParent;

               int i = boxVm.PointVms.IndexOf(origVertex);

               if (i == -1)
               {
                  return null;
               }

               LfPointViewModel vertex;

               if (i > 0)
               {
                  vertex = boxVm.PointVms[i - 1];
               }
               else
               {
                  vertex = boxVm.PointVms[boxVm.PointVms.Count() - 1];
               }

               Point p = new Point(vertex.PosX, vertex.PosY);
               Point rp = shapeVm.RotatedPointFromLocal(p);

               if (parameter as string == "x")
               {
                  return rp.X;
               }
               else
               {
                  return rp.Y;
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

   class MultiRotatedPointValueConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 2)
         {
            if ((values[0] is double) && (values[1] is LfPointViewModel))
            {
               double pos = (double)values[0];
               LfPointViewModel vertex = (LfPointViewModel)values[1];
               IBoxPointsInterface boxVm = vertex.PointsParent;
               LfShapeViewModel shapeVm = (LfShapeViewModel)vertex.PointsParent;

               Point p;

               if (parameter as string == "x")
               {
                  p = new Point(pos, vertex.PosY);
                  Point rp = shapeVm.RotatedPointFromLocal(p);
                  return rp.X;
               }
               else
               {
                  p = new Point(vertex.PosX, pos);
                  Point rp = shapeVm.RotatedPointFromLocal(p);
                  return rp.Y;
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

   class RotatedVertexXValueConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (value is LfDragablePointViewModel)
         {
            LfDragablePointViewModel vertex = (LfDragablePointViewModel)value;
            LfShapeViewModel shape = vertex.Parent;

            Point p = new Point(vertex.PosX, vertex.PosY);
            Point rp = shape.RotatedPointFromLocal(p);
            
            return rp.X;
         }

         return null;
      }

      public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }

   class RotatedVertexYValueConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (value is LfDragablePointViewModel)
         {
            LfDragablePointViewModel vertex = (LfDragablePointViewModel)value;
            LfShapeViewModel shape = vertex.Parent;

            Point p = new Point(vertex.PosX, vertex.PosY);
            Point rp = shape.RotatedPointFromLocal(p);

            return rp.Y;
         }

         return null;
      }

      public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }

   class ConditionalRotatedJointValueConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 7)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] != null) && (values[2] is LfShapeViewModel))
            {
               // In this case Shape B exists and is used. The conversion is done as in MultiRotatedJointValueConverter
               Point pos = new Point((double)values[0], (double)values[1]);
               LfShapeViewModel shape = (LfShapeViewModel)values[2];
               Point rp = shape.RotatedPointFromLocal(pos);
               rp.Offset(shape.PosX, shape.PosY);

               if (parameter as string == "x")
               {
                  return rp.X;
               }
               else
               {
                  return rp.Y;
               }
            }
            else if((values[3] is double) && (values[4] is double) && (values[5] is LfShapeViewModel) && (values[6] is double))
            {
               // In this case Shape B does not exists, the end point will be as For shape A but a distance Length below

               Point pos = new Point((double)values[3], (double)values[4]);
               LfShapeViewModel shape = (LfShapeViewModel)values[5];
               double length = (double)values[6];
               Point rp = shape.RotatedPointFromLocal(pos);
               rp.Offset(shape.PosX, shape.PosY);

               rp.Y += length; 

               if (parameter as string == "x")
               {
                  return rp.X;
               }
               else
               {
                  return rp.Y;
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

}
