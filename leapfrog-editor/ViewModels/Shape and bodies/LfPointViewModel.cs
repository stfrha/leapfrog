using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace LeapfrogEditor
{
    class LfPointViewModel : MicroMvvm.ViewModelBase, IPositionInterface
   {
      #region Declarations

//      protected bool _isSelected;
      private double _posX;
      private double _posY;

      private MainViewModel _mainVm;
      private IBoxPointsInterface _pointsParent;

      #endregion

      #region Constructors

      public LfPointViewModel(MainViewModel mainVm, IBoxPointsInterface parent, Point p)
      {
         PosX = p.X;
         PosY = p.Y;
         MainVm = mainVm;
         PointsParent = parent;
//         IsSelected = false;
      }

      #endregion

      #region Properties

      public MainViewModel MainVm
      {
         get { return _mainVm; }
         set { _mainVm = value; }
      }

      public IBoxPointsInterface PointsParent
      {
         get { return _pointsParent; }
         set
         {
            _pointsParent = value;
            OnPropertyChanged("");
         }
      }

      public double PosX
      {
         get
         {
            return _posX;
         }
         set
         {
            _posX = value;
            OnPropertyChanged("PosX");
         }
      }


      public double PosY
      {
         get
         {
            return _posY;
         }
         set
         {
            _posY = value;
            OnPropertyChanged("PosY");
         }
      }

      //public bool IsSelected
      //{
      //   get { return _isSelected; }
      //   set
      //   {
      //      _isSelected = value;
      //      OnPropertyChanged("IsSelected");
      //   }
      //}


      #endregion
   }
}
