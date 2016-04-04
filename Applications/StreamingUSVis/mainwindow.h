/*==========================================================================

  Copyright (c) 2015 Uditha L. Jayarathne, ujayarat@robarts.ca

  Use, modification and redistribution of the software, in source or
  binary forms, are permitted provided that the following terms and
  conditions are met:

  1) Redistribution of the source code, in verbatim or modified
  form, must retain the above copyright notice, this license,
  the following disclaimer, and any notices that refer to this
  license and/or the following disclaimer.

  2) Redistribution in binary form must include the above copyright
  notice, a copy of this license and the following disclaimer
  in the documentation or with other materials provided with the
  distribution.

  3) Modified copies of the source code must be clearly marked as such,
  and must not be misrepresented as verbatim copies of the source code.

  THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE SOFTWARE "AS IS"
  WITHOUT EXPRESSED OR IMPLIED WARRANTY INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE.  IN NO EVENT SHALL ANY COPYRIGHT HOLDER OR OTHER PARTY WHO MAY
  MODIFY AND/OR REDISTRIBUTE THE SOFTWARE UNDER THE TERMS OF THIS LICENSE
  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, LOSS OF DATA OR DATA BECOMING INACCURATE
  OR LOSS OF PROFIT OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF
  THE USE OR INABILITY TO USE THE SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGES.
  =========================================================================*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QSignalMapper>
#include <QMenuBar>

// VTK includes
#include <vtkBoxRepresentation.h>
#include <vtkBoxWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkColorTransferFunction.h>
#include <vtkCommand.h>
#include <vtkImageActor.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkImageClip.h>
#include <vtkImageFlip.h>
#include <vtkImageImport.h>
#include <vtkImageMapper3D.h>
#include <vtkImageViewer2.h>
#include <vtkInteractorStyleImage.h>
#include <vtkMatrix4x4.h>
#include <vtkMetaImageWriter.h>
#include <vtkPNGWriter.h>
#include <QVTKWidget.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPlanes.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRfProcessor.h>
#include <vtkSavedDataSource.h>
#include <vtkSmartPointer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTexture.h>
#include <vtkTransform.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkWindowToImageFilter.h>
#include <vtkXMLUtilities.h>

// for testing
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

// PLUS includes
#include <DataCollection/vtkPlusChannel.h>
#include <DataCollection/vtkPlusDataSource.h>
#include <DataCollection/vtkPlusDevice.h>
#include <PlusCommon/TrackedFrame.h>
#include <PlusCommon/vtkTrackedFrameList.h>
#include <PlusConfigure.h>
#include <VolumeReconstruction/vtkVolumeReconstructor.h>
#include <vtkDataCollector.h>
#include <vtkSequenceIO.h>
#include <vtkTrackedFrameList.h>
#include <vtkTransformRepository.h>

// RobartsVTK includes
#include "qTransferFunctionDefinitionWidget.h"
#include "qTransferFunctionWindowWidget.h"
#include "vtkCuda1DVolumeMapper.h"
#include "vtkCuda2DInExLogicVolumeMapper.h"
#include "vtkCuda2DTransferFunction.h"
#include "vtkCuda2DVolumeMapper.h"
#include "vtkCudaFunctionPolygonReader.h"

// OpenCV includes
#include "cv.h"
#include "highgui.h"

#include "CudaReconstruction.h"

#include <iostream>
#include <chrono> // For timing

# define D_TIMING
//# define ALIGNMENT_DEBUG

class vtkUSEventCallback : public vtkCommand
{
public:
  static vtkUSEventCallback *New()
  {
    return new vtkUSEventCallback;
  }

  virtual void Execute(vtkObject *caller, unsigned long, void*);

  vtkTrackedFrameList *trackedFrames;
  vtkTransformRepository *repository;
  vtkVolumeReconstructor *reconstructor;
  CudaReconstruction *accRecon;
  vtkImageData *usVolume;
  vtkSmartVolumeMapper *volMapper;
  vtkCuda1DVolumeMapper *cudaMapper;
  vtkCuda2DVolumeMapper *cudaMapper2;
  vtkImageViewer2 *Viewer;
  vtkImageFlip *imgFlip;
  vtkRenderWindowInteractor *Iren;
  PlusTransformName TransformName;
  vtkImageData *ImageData;
  cv::VideoCapture *_camCapture;
  int n_frames;
  vtkImageImport *_imgImport;
  vtkRenderWindow *_camRenWin;
  vtkRenderWindow *_volRenWin;
  vtkVolume *_vol;
  vtkRenderWindow *_augmentedRenWin;
  vtkRenderer *_volRenderer;
  vtkTexture *_camImgTexture;
  vtkTransform *_boxTransform;
  vtkTransform *_transform;
  vtkBoxWidget *_boxWidget;
  vtkPlanes *_boxPlanes;
  vtkCuda2DInExLogicVolumeMapper *_inExMapper;
  QVTKWidget * _screen;
  QLabel *Info;
  vtkWindowToImageFilter *_win2Img;
  vtkPNGWriter *_imgWriter;
  std::string current_mapper;
  bool sc_capture_on;
  int index;
};

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

public slots:
  void onStartButtonClick(const QString &);
  void onScanTypeRadioButtonClick(const QString &);
  void onSaveVolumeButtonClick(const QString &);
  void ontf1ButtonClick(const QString &);
  void ontf2ButtonClick(const QString &);
  void ontfInExButtonClick(const QString &);
  void onScCaptureRadioButtonClick(const QString &);

private:
  Ui::MainWindow *ui;

  /* Structure to hold camera video properties */
  struct VideoCaptureProperties
  {
    int framerate;
    int frame_width, frame_height;
    int n_frames;
  } cam_video_prop;

  std::string inputConfigFileName;
  std::string inputVideoBufferMetafile;
  std::string inputTrackerBufferMetafile;
  std::string inputTransformName;
  std::string scan_type;
  std::string video_filename;
  std::string calibration_filename;

  /* Camera/US frame rate. Viewers are updated at this rate */
  double frame_rate;

  /* ID for interactor Timer. Used to start/stop the corresponding timer */
  int interactorTimerID;

  int frame_counter;
  bool inputRepeat, streamingON;

  std::string outputDir;

  /* Camera intrinsics and distortion params. Data is read into these matrices from the XML files */
  cv::Mat intrinsics, distortion_params;

  /* Capture handle for capturing from file */
  cv::VideoCapture cam_capture;
  cv::Mat cam_frame;

  vtkMatrix4x4* matrix;
  /* Members associated with data streaming with PLUS Library */
  vtkSmartPointer< vtkXMLDataElement > configRootElement;
  vtkSmartPointer< vtkDataCollector > dataCollector;
  vtkPlusDevice* videoDevice;
  vtkPlusDevice* trackerDevice;

  /* Tracked US data is read from a SequenceMeta file into a vtkTrackedFrameList */
  vtkSmartPointer< vtkTrackedFrameList > trackedUSFrameList;

  /* Tracked US framelist used for online 3DUS reconstruction */
  vtkSmartPointer< vtkTrackedFrameList > trackedFrameList4Recon;

  vtkSmartPointer< vtkVolumeReconstructor >  volumeReconstructor;
  CudaReconstruction * acceleratedVolumeReconstructor;
  vtkSmartPointer< vtkTransformRepository > repository;
  vtkSmartPointer< vtkRenderer > camImgRenderer;
  vtkSmartPointer< vtkRenderWindow > camImgRenWin;
  vtkSmartPointer< vtkRenderWindow > augmentedRenWin;
  vtkSmartPointer< vtkImageData > reconstructedVol;
  vtkSmartPointer< vtkImageData > usImageData;
  vtkSmartPointer< vtkImageData > usVolume;
  vtkSmartPointer< vtkImageViewer2 > usViewer;
  vtkSmartPointer< vtkImageClip > usImageClip;
  vtkSmartPointer< vtkImageFlip > usImageFlip;
  vtkSmartPointer< vtkImageImport > camImgImport;
  vtkSmartPointer< vtkTexture > camImgTexture;
  vtkSmartPointer< vtkImageActor > camImgActor;
  vtkPlusChannel* aChannel;
  vtkSmartPointer< vtkRenderer > us_renderer;
  vtkSmartPointer<vtkRenderer > endo_renderer;
  vtkSmartPointer< vtkRenderWindowInteractor > interactor;
  vtkSmartPointer< vtkMetaImageWriter > metaWriter;
  vtkSmartPointer< vtkUSEventCallback > us_callback;

  /* Members for volume rendering */
  vtkSmartPointer< vtkSmartVolumeMapper > volumeMapper;
  vtkSmartPointer< vtkCuda1DVolumeMapper > cudaVolumeMapper;
  vtkSmartPointer< vtkCuda2DVolumeMapper > cuda2DVolumeMapper;
  vtkSmartPointer< vtkCuda2DTransferFunction > cuda2DTransferFun;
  vtkSmartPointer< vtkCuda2DTransferFunction > backgroundTF;
  vtkSmartPointer< vtkCudaFunctionPolygonReader > polyReader;
  vtkSmartPointer< vtkCuda2DInExLogicVolumeMapper > inExVolumeMapper;
  vtkSmartPointer< vtkBoxWidget > box;
  vtkSmartPointer< vtkTransform > boxTransform;
  vtkSmartPointer< vtkTransform > transform;
  vtkSmartPointer< vtkPlanes > boxPlanes;
  vtkSmartPointer< vtkImageCanvasSource2D > background;
  vtkSmartPointer< vtkVolumeProperty > volumeProperty;
  vtkSmartPointer< vtkPiecewiseFunction > compositeOpacity;
  vtkSmartPointer< vtkColorTransferFunction > color;
  vtkSmartPointer< vtkVolume > volume;
  vtkSmartPointer< vtkRenderer > volRenderer;
  vtkSmartPointer< vtkRenderWindow > volRenWin;
  qTransferFunctionWindowWidget * tfWidget;
  vtkSmartPointer< vtkSphereSource > sphere;
  vtkSmartPointer< vtkActor > actor;
  vtkSmartPointer< vtkWindowToImageFilter > windowToImage;
  vtkSmartPointer< vtkPNGWriter > imageWriter;

  /* Initialize PLUS pipeline */
  int init_PLUS_Pipeline();

  /* Initialize VTK pipeline */
  int init_VTK_Pipeline();

  /* Initialize OpenCV variables */
  int init_CV_Pipeline();

  /* Initialize PLUS-bypass pipeline */
  int init_PLUS_Bypass_Pipeline();

  /* Setup VTK Camera from intrinsics */
  void setup_VTK_Camera(cv::Mat, double, double, vtkCamera*);
  void setup_VTK_Camera(cv::Mat, vtkCamera*);

  /* Setup Volume Rendering Pipeline */
  void setup_VolumeRendering_Pipeline();

  /* Setup AR-Volume Rendering Pipeline */
  void setup_ARVolumeRendering_Pipeline();

  /* Setup US Volume Reconstruction Pipeline */
  int setup_VolumeReconstruction_Pipeline();

  int get_extent_from_trackedList(vtkTrackedFrameList *, vtkTransformRepository *,
                                  double spacing, int *, double *);

  void get_first_frame_position(TrackedFrame *,  vtkTransformRepository *, double *);

};

#endif // MAINWINDOW_H