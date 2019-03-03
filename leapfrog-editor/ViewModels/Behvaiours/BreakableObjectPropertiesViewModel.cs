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
   class BreakableObjectPropertiesViewModel : BehaviourViewModelBase, ISpawnObjectParentVmInterface
   {
      #region Declarations

      private BreakableObjectProperties ModelObject;

      private ObservableCollection<SpawnObjectViewModel> _spawnObjects = new ObservableCollection<SpawnObjectViewModel>();

      #endregion

      #region Constructors

      public BreakableObjectPropertiesViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm, 
         BreakableObjectProperties modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;
                  
         foreach (SpawnObject so in ModelObject.SpawnObjects)
         {
            SpawnObjectViewModel sovm = new SpawnObjectViewModel(this, parentVm, mainVm, this, so);
            SpawnObjects.Add(sovm);
         }
      }

      #endregion

      #region Properties

      public BreakableObjectProperties LocalModelObject
      {
         get { return ModelObject; }
      }

      public ISpawnObjectParentInterface SpawnParentModelObject
      {
         get { return ModelObject as ISpawnObjectParentInterface; }
      }

      public int BreakBulletDamage
      {
         get { return LocalModelObject.BreakBulletDamage; }
         set
         {
            LocalModelObject.BreakBulletDamage = value;
            OnPropertyChanged("BreakBulletDamage");
         }
      }

      public int NumberOfSpawns
      {
         get { return LocalModelObject.NumberOfSpawns; }
         set
         {
            LocalModelObject.NumberOfSpawns = value;
            OnPropertyChanged("NumberOfSpawns");
         }
      }

      public ObservableCollection<SpawnObjectViewModel> SpawnObjects
      {
         get { return _spawnObjects; }
         set
         {
            _spawnObjects = value;
            OnPropertyChanged("SpawnObjects");
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
