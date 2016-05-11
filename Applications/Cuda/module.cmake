vtk_module(Cuda2DTFVolumeRendering
  GROUPS
    Examples
  DEPENDS
    vtkRenderingVolume${VTK_RENDERING_BACKEND}  
    vtkFiltersCore 
    vtkImagingCore 
    vtkInteractionWidgets 
    vtkIOImage 
    vtkCommonCore 
    vtkCudaCommon 
    vtkCudaVisualization 
  EXCLUDE_FROM_WRAPPING
  )