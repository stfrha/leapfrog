using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

namespace LeapfrogEditor
{
   public class FileCOViewModel : CompoundObjectViewModel
   {
      #region Declarations

      // Even though this is a collection, it should only contain one object, the top level 
      // CompoundObject of the file.
      private ObservableCollection<CompoundObjectViewModel> _topObject = new ObservableCollection<CompoundObjectViewModel>();

      private string _fileName;

      #endregion

      #region Constructors

      public FileCOViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         string fileName,
         CompoundObject modelObject) :
         base(treeParent, parentVm, mainVm, modelObject)
      {
         _fileName = fileName;
      }

      #endregion

      #region Properties

      public string FileName
      {
         get { return _fileName; }
         set
         {
            if (value != _fileName)
            {
               _fileName = value;
               OnPropertyChanged("FileName");
               OnPropertyChanged("FullPathFileName");
            }
         }
      }

      public string FullPathFileName
      {
         get
         {
            string s = GlobalConstants.DataDirectory + FileName;
            string fullPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
            string fullFileName = System.IO.Path.Combine(fullPath, s);

            return fullFileName;
         }
      }

      // Even though this is a collection, it should only contain one object, the top level 
      // CompoundObject of the file.
      public ObservableCollection<CompoundObjectViewModel> TopObject
      {
         get { return _topObject; }
         set { _topObject = value; }
      }

      #endregion

      #region Private Methods


      #endregion

      #region Public Methods


      #endregion

   }
}
