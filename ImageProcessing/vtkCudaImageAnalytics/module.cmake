vtk_module(vtkCudaImageAnalytics
  GROUPS
    Cuda
    Imaging
  DEPENDS
    vtkRobartsCommon
    vtkCudaCommon
    vtkFiltersCore
    vtkCommonCore
  KIT
    vtkRobartsCuda
  EXCLUDE_FROM_WRAP_HIERARCHY
  )