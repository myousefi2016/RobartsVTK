#ifndef __VTKIMAGEMULTISTATISTICS_H__
#define __VTKIMAGEMULTISTATISTICS_H__

#include "vtkProcessObject.h"
#include "vtkImageData.h"
#include "vtkInformation.h"

class vtkImageMultiStatistics : public vtkProcessObject
{
public:
  static vtkImageMultiStatistics *New();

  vtkTypeMacro(vtkImageMultiStatistics,vtkProcessObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Compute and return the average magnitude.
  double GetAverageMagnitude(int component = 0);
  
  // Description:
  // Compute and return the standard deviation.
  double GetStandardDeviation(int component = 0);

  // Description:
  // Compute and return the covariance.
  double GetCovariance(int component1 = 0, int component2 = 0);

  // Description:
  // Compute and return the entropy of the image in terms of a given component.
  double GetSingleEntropy(int component = 0);

  // Description:
  // Compute and return the joint entropy of the image in terms of the given components
  double GetJointEntropy(int component1 = 0, int component2 = 0);

  // Description:
  // Compute and return the number of pixels.
  long int GetCount();

  // Description:
  // Set the number of bins used in PDF estimation for entropy
  void SetEntropyResolution(int bins);
  int GetEntropyResolution();

  void Update();
  
  void SetInput(int port, vtkImageData *input);
  void SetInput(vtkImageData *input) { SetInput(0,input); }
  vtkImageData *GetInput(int port);
  vtkImageData *GetInput(){ return GetInput(0); };

  int FillInputPortInformation(int i, vtkInformation* info);

protected:
  vtkImageMultiStatistics();
  ~vtkImageMultiStatistics();

  void Execute();
  
  long double* AverageMagnitude;
  long double* Covariance;
  long double* JointEntropy;
  long int Count;
  int NumberOfComponents;
  int NumberOfBins;

  vtkTimeStamp ExecuteTime;

private:
  vtkImageMultiStatistics(const vtkImageMultiStatistics&) {}; //not implemented
  void operator=(const vtkImageMultiStatistics&) {}; //not implemented
};

#endif \\__VTKIMAGEMULTISTATISTICS_H__


