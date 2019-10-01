﻿using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace LeapfrogEditor
{
   class LfSpritePolygonViewModel : LfScalableTexturePolygonViewModel
   {
      #region Declarations

      #endregion

      #region Constructors

      public LfSpritePolygonViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm, 
         LfSpritePolygon modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;

         //LfSpritePolygon modelSP = ModelObject as LfSpritePolygon;

         //foreach (LfDragablePoint dragPoint in modelSP.Points)
         //{
         //   LfDragablePointViewModel dragPointVm = new LfDragablePointViewModel(this, parentVm, MainVm, this, dragPoint, enabled);
         //   PointVms.Add(dragPointVm);
         //}

      }

      #endregion

      #region Properties

      public new LfSpritePolygon LocalModelObject
      {
         get { return (LfSpritePolygon)ModelObject; }
      }

      #endregion

      #region public methods

      #endregion
   }
}
