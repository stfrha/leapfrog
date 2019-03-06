using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace LeapfrogEditor
{
   /// <summary>
   /// Interaction logic for CompoundObjectUserControl.xaml
   /// </summary>
   public partial class CompoundObjectUserControl : UserControl
   {
      /// <summary>
      /// Specifies the current state of the mouse handling logic.
      /// </summary>
      private ZoomAndPan.MouseHandlingMode mouseHandlingMode = ZoomAndPan.MouseHandlingMode.None;

      /// <summary>
      /// The point that was clicked relative to the content that is contained within the ZoomAndPanControl.
      /// </summary>
      private Point origContentMouseDownPoint;

      /// <summary>
      /// Records which mouse button clicked during mouse dragging.
      /// </summary>
      private MouseButton mouseButtonDown;



      public CompoundObjectUserControl()
      {
         InitializeComponent();

         //CollectionView myCollectionView = (CollectionView)CollectionViewSource.GetDefaultView(jointItems.Items);
         //((INotifyCollectionChanged)myCollectionView).CollectionChanged += new NotifyCollectionChangedEventHandler(DataGrid_CollectionChanged);

         //var dpd = DependencyPropertyDescriptor.FromProperty(ItemsControl.ItemsSourceProperty, typeof(ItemsControl));
         //if (dpd != null)
         //{
         //   dpd.AddValueChanged(jointItems, ThisIsCalledWhenPropertyIsChanged);
         //}
      }



      public bool EnableContextMenues
      {
         get { return (bool)GetValue(EnableContextMenuesProperty); }
         set { SetValue(EnableContextMenuesProperty, value); }
      }

      // Using a DependencyProperty as the backing store for EnableContextMenues.  This enables animation, styling, binding, etc...
      public static readonly DependencyProperty EnableContextMenuesProperty =
          DependencyProperty.Register("EnableContextMenues", typeof(bool), typeof(CompoundObjectUserControl), new PropertyMetadata(true));

      
      public bool ShowJoints
      {
         get { return (bool)GetValue(ShowJointsProperty); }
         set { SetValue(ShowJointsProperty, value); }
      }

      // Using a DependencyProperty as the backing store for ShowJoints.  This enables animation, styling, binding, etc...
      public static readonly DependencyProperty ShowJointsProperty =
          DependencyProperty.Register("ShowJoints", typeof(bool), typeof(CompoundObjectUserControl), new PropertyMetadata(false));

      public bool ShowSystems
      {
         get { return (bool)GetValue(ShowSystemsProperty); }
         set { SetValue(ShowSystemsProperty, value); }
      }

      // Using a DependencyProperty as the backing store for ShowSystems.  This enables animation, styling, binding, etc...
      public static readonly DependencyProperty ShowSystemsProperty =
          DependencyProperty.Register("ShowSystems", typeof(bool), typeof(CompoundObjectUserControl), new PropertyMetadata(false));




      public bool ShowTriangles
      {
         get { return (bool)GetValue(ShowTrianglesProperty); }
         set { SetValue(ShowTrianglesProperty, value); }
      }

      // Using a DependencyProperty as the backing store for ShowTriangles.  This enables animation, styling, binding, etc...
      public static readonly DependencyProperty ShowTrianglesProperty =
          DependencyProperty.Register("ShowTriangles", typeof(bool), typeof(CompoundObjectUserControl), new PropertyMetadata(false));







      public void GeneralMouse<T>(bool down, MouseEventObjectType type, object sender, MouseButtonEventArgs e)
      {
         // We want access to the parent Canvas above the "myCoUserControl" Canvas. 
         // The "myCoUserControl" (content it is it's name) Canvas, is the top Canvas of the 
         // CompoundObjectUserControl, so the parent canvas is the one above
         // it. We need it to set the focus (why?) and to get the mouse point
         // expressed in the parent coordinate system.
         // Hence, we are not seeking the top most canvas, i.e. the CenteredCanvas
         // specialised Canvas of the main window. 
         // By getting access to the "myCoUserControl" Canvas which is the top most canvas
         // of this object, we search immediatly above it to make sure that the found
         // canvas is the one above it. 
         // Now, can this be done without knowledge of the myCoUserControl Canvas?
         // Can it, for instance, be assumed that the myCoUserControl Canvas is the first 
         // Parent Canvas and the sought Canvas is the second found?
         // 
         UIElement parentCanvas = ParentalFinder.FindParent<Canvas>(myCoUserControl);
         parentCanvas.Focus();
         Keyboard.Focus(parentCanvas);

         mouseButtonDown = e.ChangedButton;

         bool shift = ((Keyboard.Modifiers & ModifierKeys.Shift) != 0);
         bool ctrl = ((Keyboard.Modifiers & ModifierKeys.Control) != 0);
         bool alt = ((Keyboard.Modifiers & ModifierKeys.Alt) != 0);

         // If the leftClickState of the Main VM is other than none then 
         // we do not want to start dragging objects
         if (sender is FrameworkElement)
         {
            FrameworkElement fwe = (FrameworkElement)sender;

            if (fwe.DataContext is T)
            {
               IMainVmInterface obj = (IMainVmInterface)fwe.DataContext;

               if (obj.MainVm.LeftClickState == LeftClickState.none)
               {
                  // If ctrl is held down, we never intent to drag things
                  if (!ctrl && (mouseButtonDown == MouseButton.Left))
                  {
                     if (down)
                     {
                        mouseHandlingMode = ZoomAndPan.MouseHandlingMode.DraggingObjects;
                        origContentMouseDownPoint = e.GetPosition(parentCanvas);

                        UIElement dragObject = (UIElement)sender;
                        dragObject.CaptureMouse();

                        e.Handled = true;
                     }
                     else
                     {
                        if (mouseHandlingMode == ZoomAndPan.MouseHandlingMode.DraggingObjects)
                        {
                           mouseButtonDown = e.ChangedButton;

                           if (mouseButtonDown == MouseButton.Left)
                           {
                              mouseHandlingMode = ZoomAndPan.MouseHandlingMode.None;

                              UIElement rectangle = (UIElement)sender;
                              rectangle.ReleaseMouseCapture();

                              e.Handled = true;
                           }
                        }
                     }
                  }
               }

               if (down)
               {
                  if (obj.MainVm.MouseDown(type, obj, e.ChangedButton, e.GetPosition(myCoUserControl), e.ClickCount, shift, ctrl, alt))
                  {
                     e.Handled = true;
                  }
               }
               else
               {
                  if (obj.MainVm.MouseUp(type, obj, e.ChangedButton, e.GetPosition(myCoUserControl), e.ClickCount, shift, ctrl, alt))
                  {
                     e.Handled = true;
                  }
               }
            }
         }
      }

      public void GeneralMouseMove<T>(MouseEventObjectType type, object sender, MouseEventArgs e)
      {
         if (mouseHandlingMode == ZoomAndPan.MouseHandlingMode.DraggingObjects)
         {
            if (mouseButtonDown == MouseButton.Left)
            {
               UIElement parentCanvas = ParentalFinder.FindParent<Canvas>(myCoUserControl);

               Point curContentPoint = e.GetPosition(parentCanvas);
               Vector rectangleDragVector = curContentPoint - origContentMouseDownPoint;

               origContentMouseDownPoint = curContentPoint;

               bool shift = ((Keyboard.Modifiers & ModifierKeys.Shift) != 0);
               bool ctrl = ((Keyboard.Modifiers & ModifierKeys.Control) != 0);
               bool alt = ((Keyboard.Modifiers & ModifierKeys.Alt) != 0);

               if (sender is FrameworkElement)
               {
                  FrameworkElement fwe = (FrameworkElement)sender;

                  if (fwe.DataContext is IMainVmInterface)
                  {
                     IMainVmInterface mvm = (IMainVmInterface)fwe.DataContext;

                     T obj = (T)fwe.DataContext;

                     mvm.MainVm.MouseMove(type, obj, rectangleDragVector, shift, ctrl, alt);
                  }
               }

               e.Handled = true;

            }
         }
      }

      private void ShapeBorder_MouseDown(object sender, MouseButtonEventArgs e)
      {
         GeneralMouse<LfPointViewModel>(true, MouseEventObjectType.dragableBorder, sender, e);
      }

      private void ShapeBorder_MouseUp(object sender, MouseButtonEventArgs e)
      {
         GeneralMouse<LfPointViewModel>(false, MouseEventObjectType.dragableBorder, sender, e);
      }

      private void ShapeBorder_MouseMove(object sender, MouseEventArgs e)
      {
         GeneralMouseMove<IPositionInterface>(MouseEventObjectType.dragableBorder, sender, e);
      }

      private void CompoundObject_MouseDown(object sender, MouseButtonEventArgs e)
      {
         GeneralMouse<CompoundObjectViewModel>(true, MouseEventObjectType.compoundObjectBoundaryBox, sender, e);
      }

      private void CompoundObject_MouseUp(object sender, MouseButtonEventArgs e)
      {
         GeneralMouse<CompoundObjectViewModel>(false, MouseEventObjectType.compoundObjectBoundaryBox, sender, e);
      }

      private void CompoundObject_MouseMove(object sender, MouseEventArgs e)
      {
         GeneralMouseMove<IPositionInterface>(MouseEventObjectType.compoundObjectBoundaryBox, sender, e);
      }

      private void OnContextMenuOpening(object sender, ContextMenuEventArgs e)
      {
         // By not doing anything here, can we prevent the context menu?
      }

      //private void OnDataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
      //{
      //   // Here I want to disable the Context menu of all children
      //   for (int i = 0; i < jointItems.Items.Count; i++)
      //   {
      //      UIElement element = jointItems.ItemContainerGenerator.ContainerFromIndex(i) as UIElement;

      //      int a = 10;
      //   }

      //}

      //private void OnSourceUpdated(object sender, DataTransferEventArgs e)
      //{
      //   // Here I want to disable the Context menu of all children
      //   for (int i = 0; i < jointItems.Items.Count; i++)
      //   {
      //      UIElement element = jointItems.ItemContainerGenerator.ContainerFromIndex(i) as UIElement;

      //      int a = 10;
      //   }
      //}

      //private void ThisIsCalledWhenPropertyIsChanged(object sender, EventArgs e)
      //{
      //   // Here I want to disable the Context menu of all children

      //   for (int i = 0; i < jointItems.Items.Count; i++)
      //   {
      //      var t = jointItems.ItemContainerGenerator.ContainerFromIndex(i);

      //      UIElement element = jointItems.ItemContainerGenerator.ContainerFromIndex(i) as UIElement;

      //      int a = 10;
      //   }
      //}

      //private void DataGrid_CollectionChanged(object sender, EventArgs e)
      //{
      //   // Here I want to disable the Context menu of all children
      //   for (int i = 0; i < jointItems.Items.Count; i++)
      //   {
      //      var t = jointItems.ItemContainerGenerator.ContainerFromIndex(i);
      //      if (t == null)
      //      {
      //         jointItems.UpdateLayout();
      //         t = jointItems.ItemContainerGenerator.ContainerFromIndex(i);
      //      }
      //      if (t is FrameworkElement)
      //      {
      //         FrameworkElement element = t as FrameworkElement;

      //         element.ContextMenu = null;

      //         int a = 10;

      //      }
      //   }
      //}
   }
}
