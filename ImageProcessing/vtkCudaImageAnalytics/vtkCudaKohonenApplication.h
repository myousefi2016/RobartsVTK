#ifndef __VTKCUDAKOHONENAPPLICATION_H__
#define __VTKCUDAKOHONENAPPLICATION_H__

#include "vtkCudaImageAnalyticsExport.h"
#include "vtkVersionMacros.h"

#include "CUDA_kohonenapplication.h"
#include "CudaObject.h"
#include "vtkImageAlgorithm.h"

class vtkAlgorithmOutput;
class vtkImageCast;
class vtkImageData;
class vtkInformation;
class vtkInformationVector;
class vtkTransform;

class vtkCudaImageAnalyticsExport vtkCudaKohonenApplication : public vtkImageAlgorithm, public CudaObject
{
public:
  vtkTypeMacro(vtkCudaKohonenApplication, vtkImageAlgorithm);

  static vtkCudaKohonenApplication* New();

  void SetScale(double s);
  double GetScale();
  void SetDataInputData(vtkImageData* d);
  void SetMapInputData(vtkImageData* d);
  void SetDataInputConnection(vtkAlgorithmOutput* d);
  void SetMapInputConnection(vtkAlgorithmOutput* d);

  vtkImageData* GetDataInput();
  vtkImageData* GetMapInput();

  // Description:
  // If the subclass does not define an Execute method, then the task
  // will be broken up, multiple threads will be spawned, and each thread
  // will call this method. It is public so that the thread functions
  // can call this method.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);
  virtual int RequestInformation(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);
  virtual int RequestUpdateExtent(vtkInformation* request,
                                  vtkInformationVector** inputVector,
                                  vtkInformationVector* outputVector);
  virtual int FillInputPortInformation(int i, vtkInformation* info);

protected:
  vtkCudaKohonenApplication();
  virtual ~vtkCudaKohonenApplication();

  virtual void Reinitialize(bool withData = false);
  virtual void Deinitialize(bool withData = false);

  double Scale;

  Kohonen_Application_Information Info;

private:
  vtkCudaKohonenApplication operator=(const vtkCudaKohonenApplication&);
  vtkCudaKohonenApplication(const vtkCudaKohonenApplication&);
};

#endif