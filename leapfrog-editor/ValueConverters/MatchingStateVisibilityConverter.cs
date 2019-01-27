using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;

namespace LeapfrogEditor
{
   class MatchingStateVisibilityConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 2)
         {
            if ((values[0] is string) && (values[1] is string))
            {
               string objState = (string)values[0];
               string dispState = (string)values[1];

               if ((dispState == objState) || (objState == "default"))
               {
                  return Visibility.Visible;
               }
               return Visibility.Collapsed;
            }
         }

         return null;
      }
      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         throw new NotImplementedException();
      }
   }


   //class MatchingStateVisibilityConverter : IValueConverter
   //{
   //   public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
   //   {
   //      if (value is CompoundObjectViewModel)
   //      {
   //         CompoundObjectViewModel vm = (CompoundObjectViewModel)value;
   //         ChildObjectStatePropertiesViewModel cvm = vm.TreeParent as ChildObjectStatePropertiesViewModel;

   //         if((vm.MainVm.GetEditableCoBehaviourState().StateName == cvm.State) || (cvm.State == "default"))
   //         {
   //            return Visibility.Visible;
   //         }
   //         return Visibility.Collapsed;
   //      }

   //      return null;

   //   }

   //   public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
   //   {
   //      return DependencyProperty.UnsetValue;
   //   }
   //}

}
