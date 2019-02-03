using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace LeapfrogEditor
{
   class ExplosiveObjectPropertiesViewModel : BehaviourViewModelBase
   {
      #region Declarations

      private ExplosiveObjectProperties ModelObject;

      #endregion

      #region Constructors

      public ExplosiveObjectPropertiesViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm,
         ExplosiveObjectProperties modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;
                  
      }

      #endregion

      #region Properties

      public ExplosiveObjectProperties LocalModelObject
      {
         get { return ModelObject; }
      }

      public int NumOfParticles
      {
         get { return LocalModelObject.NumOfParticles; }
         set
         {
            LocalModelObject.NumOfParticles = value;
            OnPropertyChanged("NumOfParticles");
         }
      }

      public double ParticlesRadius
      {
         get { return LocalModelObject.ParticlesRadius; }
         set
         {
            LocalModelObject.ParticlesRadius = value;
            OnPropertyChanged("ParticlesRadius");
         }
      }

      public double BlastPower
      {
         get { return LocalModelObject.BlastPower; }
         set
         {
            LocalModelObject.BlastPower = value;
            OnPropertyChanged("BlastPower");
         }
      }

      #endregion

      #region private Methods

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      #endregion
   }
}
