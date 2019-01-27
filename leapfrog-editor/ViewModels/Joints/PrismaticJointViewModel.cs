using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace LeapfrogEditor
{
   class PrismaticJointViewModel : MotorizedJointViewModel, IMainVmInterface
   {
      #region Declarations

      private PrismaticJoint _modelObject;

      #endregion

      #region Constructors

      public PrismaticJointViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm, 
         PrismaticJoint modelObject,
         bool enabled = true) : 
         base(treeParent, parentVm, mainVm, modelObject, enabled)
      {
         ModelObject = modelObject;

         //_aVm = Parent.FindShape(ModelObject.AName);
         //if (_aVm == null)
         //{
         //   MessageBox.Show("The shape A pointed to by " + ModelObject.Name + " does not exists in CO " + Parent.Name, "Error parsing file", MessageBoxButton.OK, MessageBoxImage.Error);
         //}

         //_bVm = Parent.FindShape(ModelObject.BName);
         //if (_bVm == null)
         //{
         //   MessageBox.Show("The shape B pointed to by " + ModelObject.Name + " does not exists in CO " + Parent.Name, "Error parsing file", MessageBoxButton.OK, MessageBoxImage.Error);
         //}
      }

      #endregion

      #region Properties

      public new PrismaticJoint ModelObject
      {
         get
         {
            if (_modelObject == null) return null;
            return _modelObject;
         }
         set
         {
            _modelObject = value;
            OnPropertyChanged("");
         }
      }

      public double AAxisX
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.AAxisX;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.AAxisX = value;
            OnPropertyChanged("AAxisX");
            OnPropertyChanged("UpperLimitPosX");
            OnPropertyChanged("UpperLimitPosY");
            OnPropertyChanged("LowerLimitPosX");
            OnPropertyChanged("LowerLimitPosY");
         }
      }

      public double AAxisY
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.AAxisY;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.AAxisY = value;
            OnPropertyChanged("AAxisY");
            OnPropertyChanged("UpperLimitPosX");
            OnPropertyChanged("UpperLimitPosY");
            OnPropertyChanged("LowerLimitPosX");
            OnPropertyChanged("LowerLimitPosY");
         }
      }

      public double RefAngle
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.RefAngle;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.RefAngle = value;
            OnPropertyChanged("RefAngle");
         }
      }

      public double LowerLimit
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.LowerLimit;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.LowerLimit = value;
            OnPropertyChanged("LowerLimit");
            OnPropertyChanged("LowerLimitPosX");
            OnPropertyChanged("LowerLimitPosY");
         }
      }

      public double UpperLimit
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.UpperLimit;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.UpperLimit = value;
            OnPropertyChanged("UpperLimit");
            OnPropertyChanged("UpperLimitPosX");
            OnPropertyChanged("UpperLimitPosY");
         }
      }

      public double UpperLimitPosX
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.AAxisX * _modelObject.UpperLimit + _modelObject.AAnchorX;

            //double localPosX = _modelObject.AAxisX * _modelObject.UpperLimit;
            //double localPosY = _modelObject.AAxisY * _modelObject.UpperLimit;

            //Point p = _aVm.RotatedPointFromLocal(new Point(localPosX, localPosY));

            //return p.X;
         }
      }

      public double UpperLimitPosY
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.AAxisY * _modelObject.UpperLimit + _modelObject.AAnchorY;

            //double localPosX = _modelObject.AAxisX * _modelObject.UpperLimit;
            //double localPosY = _modelObject.AAxisY * _modelObject.UpperLimit;

            //Point p = _aVm.RotatedPointFromLocal(new Point(localPosX, localPosY));

            //return p.Y;
         }
      }

      public double LowerLimitPosX
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.AAxisX * _modelObject.LowerLimit + _modelObject.AAnchorX;

            //double localPosX = -_modelObject.AAxisX * _modelObject.LowerLimit;
            //double localPosY = -_modelObject.AAxisY * _modelObject.LowerLimit;

            //Point p = _aVm.RotatedPointFromLocal(new Point(localPosX, localPosY));

            //return p.X;
         }
      }

      public double LowerLimitPosY
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.AAxisY * _modelObject.LowerLimit + _modelObject.AAnchorY;

            //double localPosX = -_modelObject.AAxisX * _modelObject.LowerLimit;
            //double localPosY = -_modelObject.AAxisY * _modelObject.LowerLimit;

            //Point p = _aVm.RotatedPointFromLocal(new Point(localPosX, localPosY));

            //return p.Y;
         }
      }

      public new double AAnchorX
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.AAnchorX;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.AAnchorX = value;
            OnPropertyChanged("AAnchorX");
            OnPropertyChanged("UpperLimitPosX");
            OnPropertyChanged("UpperLimitPosY");
            OnPropertyChanged("LowerLimitPosX");
            OnPropertyChanged("LowerLimitPosY");
         }
      }

      public new double AAnchorY
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.AAnchorY;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.AAnchorY = value;
            OnPropertyChanged("AAnchorY");
            OnPropertyChanged("UpperLimitPosX");
            OnPropertyChanged("UpperLimitPosY");
            OnPropertyChanged("LowerLimitPosX");
            OnPropertyChanged("LowerLimitPosY");
         }
      }

      public double MaxMotorForce
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.MaxMotorForce;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.MaxMotorForce = value;
            OnPropertyChanged("MaxMotorForce");
         }
      }

      #endregion

      #region public Methods

      #endregion
   }
}
