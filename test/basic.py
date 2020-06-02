import numpy as np

DATASET_PATH = '/home/tim/Dataset/jetbot3/'


# load_camera load camera parameters
# retval 
def load_camera(path):
    with open(path) as f:
        l = f.readline()

        l = l.split()

        fx = l[0]
        fy = l[1]
        cx = l[2]
        cy = l[3]

        K = np.asarray([fx, 0, cx, 0, fy, cy, 0, 0, 1], dtype=np.float32).reshape(3, 3)
        coef = np.asarray(l[4:8]+[0], dtype=np.float32)
    return K, coef


def test_install():
    from dso_python import DirectSparseOdometry

def test_dso_creation(): 
    from dso_python import DirectSparseOdometry
    dso = DirectSparseOdometry(False)

def test_undistort():
    import cv2
    img = cv2.imread("%s/images/0000.jpg"%DATASET_PATH)
    cv2.imshow('original', img)
    cv2.waitKey(1000)
    cv2.destroyAllWindows()

    K, coef = load_camera("%s/camera.txt"%(DATASET_PATH))

    img2 = cv2.undistort(img, cameraMatrix=K, distCoeffs=coef)
    cv2.imshow('undistorted.png', img2)
    cv2.waitKey(1000)
    cv2.destroyAllWindows()

def test_builtin_undistort():
    from dso_python import Undistort
    import cv2
    distorter = Undistort.getUndistorterForFile("%s/camera.txt"%(DATASET_PATH), "", "")

    file_name = "%s/images/%04d.jpg"%(DATASET_PATH, 0)
    img = cv2.imread(file_name, cv2.IMREAD_GRAYSCALE)

    img = distorter.undistort(img)
    print(img)

    cv2.imwrite('result.png', img)
    

def dso_feed(gui=True):
    import dso_python
    from dso_python import DirectSparseOdometry, Undistort
    import cv2
    import threading
    K, coef = load_camera("%s/camera.txt"%(DATASET_PATH))
    dso_python.setCalibration(K, 960, 540)
    dso = DirectSparseOdometry(gui)

    def func(obj):
        import sys
        import os.path
        for i in range(1000):
            file_name = "%s/images/%04d.jpg"%(DATASET_PATH, i)
            if not os.path.isfile(file_name):
                break
            img = cv2.imread(file_name, cv2.IMREAD_GRAYSCALE)
            img = cv2.undistort(img, cameraMatrix=K, distCoeffs=coef).astype(np.float32)
   
            #cv2.imshow('current', img/255)
            cv2.waitKey(1)
            dso.addActiveFrame(img, i)
            
            if dso.initFailed:
                break

        dso.waitForMapping()

    gui = threading.Thread(target=func, args=(dso,))
    gui.start()

    if gui:
        dso.run()
    gui.join()


def dso_builtin_distorter(gui=True):
    import dso_python
    from dso_python import DirectSparseOdometry, Undistort
    import cv2
    import threading
    distorter = Undistort.getUndistorterForFile("%s/camera.txt"%(DATASET_PATH), "", "")
    K = distorter.K
    dso_python.setCalibration(K, distorter.width, distorter.height)
    dso = DirectSparseOdometry(gui)

    def func(obj):
        import sys
        import os.path
        for i in range(1000):
            file_name = "%s/images/%04d.jpg"%(DATASET_PATH, i)
            if not os.path.isfile(file_name):
                break
            img = cv2.imread(file_name, cv2.IMREAD_GRAYSCALE)
            img = distorter.undistort(img)
   
            cv2.imshow('current', img/255)
            cv2.waitKey(1)
            dso.addActiveFrame(img, i)
            
            if dso.initFailed:
                break

        dso.waitForMapping()

    gui = threading.Thread(target=func, args=(dso,))
    gui.start()

    dso.run()
    gui.join()



if __name__ == '__main__':
    #test_builtin_undistort()
    #test_undistort()
    #test_dso_creation()
    dso_feed(False)
    pass
