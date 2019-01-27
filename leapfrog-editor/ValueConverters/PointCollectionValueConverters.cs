using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

namespace LeapfrogEditor
{
   class ClosedPointsCollectionValueConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (value is ObservableCollection<LfDragablePointViewModel>)
         {
            ObservableCollection<LfDragablePointViewModel> _pointVms = (ObservableCollection<LfDragablePointViewModel>)value;

            // Copy points collection
            ObservableCollection<LfDragablePointViewModel> tc = new ObservableCollection<LfDragablePointViewModel>(_pointVms);

            // Add first item to end to get a closed path
            tc.Add(tc[0]);

            return tc;
         }

         return null;
      }

      public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }

   class PointCollectionValueConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (value is ObservableCollection<LfDragablePointViewModel>)
         {
            ObservableCollection<LfDragablePointViewModel> _pointVms = (ObservableCollection<LfDragablePointViewModel>)value;

            PointCollection p = new PointCollection();

            foreach (LfDragablePointViewModel dp in _pointVms)
            {
               p.Add(new Point(dp.PosX, dp.PosY));
            }

            return p;
         }

         return null;
      }

      public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }
}
