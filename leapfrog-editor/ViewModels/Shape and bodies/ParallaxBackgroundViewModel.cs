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
   // Used to be ConditionalSelectTreeViewViewModel
   public class ParallaxBackgroundViewModel : MicroMvvm.ViewModelBase, IPositionInterface
   {
      #region Declarations

      private MainViewModel _mainVm;
      private ParallaxBackgroundPropertiesViewModel _parentBackgroundVm;
      private ChildCOViewModel _parentCompoundObjectVm;
      private CompositeCollection _shapes;

      #endregion

      #region Constructors

      public ParallaxBackgroundViewModel(
         ChildCOViewModel parentVm, 
         MainViewModel mainVm,
         ParallaxBackgroundPropertiesViewModel pbpvm)
      {
         _parentCompoundObjectVm = parentVm;
         _mainVm = mainVm;
         _parentBackgroundVm = pbpvm;

         _shapes = new CompositeCollection()
         {
            new CollectionContainer { Collection = new ObservableCollection<LfSpriteBoxViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfSpritePolygonViewModel>() },
         };

         foreach (Object o in parentVm.ShapeCollection.Shapes)
         {
            if (o is LfSpriteBoxViewModel)
            {
               LfSpriteBoxViewModel svm = o as LfSpriteBoxViewModel;
               _shapes.Add(svm);
            }
            else if (o is LfSpritePolygonViewModel)
            {
               LfSpritePolygonViewModel svm = o as LfSpritePolygonViewModel;
               _shapes.Add(svm);
            }
         }
      }

      #endregion

      #region Properties

      public MainViewModel MainVm
      {
         get { return _mainVm; }
         set { _mainVm = value; }
      }

      public ChildCOViewModel ParentCompoundObjectVm
      {
         get { return _parentCompoundObjectVm; }
      }

      public double PosX
      {
         get { return 0; }
         set { }
      }


      public double AbsPosX
      {
         get { return 0; }
      }

      public double PosY
      {
         get { return 0; }
         set { }
      }


      public double AbsPosY
      {
         get { return 0; }
      }


      public uint ZLevel
      {
         get
         {
            return (uint)((1.0 - _parentBackgroundVm.ParallaxAmount) * 100.0);
         }
      }

      public CompositeCollection Shapes
      {
         get { return _shapes; }
         set { _shapes = value; }
      }

      public double Width
      {
         get
         {
            if (_parentCompoundObjectVm.Behaviour.BehaviourProperties is ScenePropertiesViewModel)
            {
               ScenePropertiesViewModel spvm = _parentCompoundObjectVm.Behaviour.BehaviourProperties as ScenePropertiesViewModel;

               return spvm.Width;
            }

            return 0;
         }
      }

      public double Height
      {
         get
         {
            if (_parentCompoundObjectVm.Behaviour.BehaviourProperties is ScenePropertiesViewModel)
            {
               ScenePropertiesViewModel spvm = _parentCompoundObjectVm.Behaviour.BehaviourProperties as ScenePropertiesViewModel;

               return spvm.Height;
            }

            return 0;
         }
      }

      #endregion

      #region protected Methods

      #endregion

      #region public Methods


      #endregion
   }
}
