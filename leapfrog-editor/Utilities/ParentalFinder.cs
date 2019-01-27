using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace LeapfrogEditor
{
   class ParentalFinder
   {
      public static T FindParent<T>(DependencyObject child) where T : DependencyObject
         // From: https://www.infragistics.com/community/blogs/b/blagunas/posts/find-the-parent-control-of-a-specific-type-in-wpf-and-silverlight
      {
         //get parent item
         DependencyObject parentObject = VisualTreeHelper.GetParent(child);

         //we've reached the end of the tree
         if (parentObject == null) return null;

         //check if the parent matches the type we're looking for
         T parent = parentObject as T;
         if (parent != null)
            return parent;
         else
            return FindParent<T>(parentObject);
      }

      public static T FindParentWithName<T>(DependencyObject child, string name) where T : DependencyObject
         // From: https://www.infragistics.com/community/blogs/b/blagunas/posts/find-the-parent-control-of-a-specific-type-in-wpf-and-silverlight
      {
         //get parent item
         DependencyObject parentObject = VisualTreeHelper.GetParent(child);

         //we've reached the end of the tree
         if (parentObject == null) return null;

         //check if the parent matches the type we're looking for
         T parent = parentObject as T;
         if (parent != null)
         {
            if (parent is FrameworkElement)
            {
               FrameworkElement fe = parent as FrameworkElement;

               if (fe.Name == name)
                  return parent;
            }
            else
            {
               // If we get here, the parent is not of the visual tree
               // and there is something wrong, return null.
               return null;
            }
         }


         // If we did not find the sought parent, and did not reach 
         // the very top, we test the parent's parent.
         return FindParent<T>(parentObject);
      }
   }
}
