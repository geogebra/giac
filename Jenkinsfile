def crosscompilers = '/var/lib/jenkins/cross-compilers'

pipeline {
  agent none

  stages {
    stage('Win') {
      agent {label 'winbuild'}
      environment {
        MAVEN = credentials('maven-repo')
      }
      steps {
        bat "C:\\msys64\\usr\\bin\\env.exe MSYSTEM=CLANG32 C:\\msys64\\usr\\bin\\bash -l -c \"cd ${env.WORKSPACE.replace('\\','/').replace('C:','/c')}; bash ./recompile-msys.sh cbuild\""
        bat "C:\\msys64\\usr\\bin\\env.exe MSYSTEM=CLANG64 C:\\msys64\\usr\\bin\\bash -l -c \"cd ${env.WORKSPACE.replace('\\','/').replace('C:','/c')}; bash ./recompile-msys.sh cbuild64\""
        bat './gradlew javagiacWin32JarClang --info'
        stash name: "giac-clang", includes: "cbuild*/**"
      }
    }
    stage('Build') {
      parallel {
        stage('Java and JS') {
          agent {label 'deploy2'}
          environment {
            MAVEN = credentials('maven-repo')
            MAC = credentials('mac-giac')
            NPM = credentials('npm-registry')
            ANDROID_SDK_ROOT='/var/lib/jenkins/.android-sdk'
            BINARYEN="${env.WORKSPACE}/emsdk/upstream"
            EMSDK_PYTHON='/usr/bin/python3.8'
            PATH="$crosscompilers/x86/bin:$crosscompilers/x86_64/bin:$crosscompilers/arm/bin:$crosscompilers/arm64/bin:$PATH"
          }
          steps {
            unstash name: "giac-clang"
            sh '''
               export SVN_REVISION=`git log -1 | grep "\\S" | tail -n 1 | sed "s/.*@\\([0-9]*\\).*/\\1/"`
              ./gradlew downloadEmsdk installEmsdk activateEmsdk
              ./gradlew :emccClean :giac-gwt:publish --no-daemon -Prevision=$SVN_REVISION --info --refresh-dependencies
              ./gradlew :updateGiac --no-daemon -Prevision=$SVN_REVISION --info'''
          }
        }
        stage('Objective C') {
          agent {label 'mac'}
          environment {
            MAVEN = credentials('maven-repo')
          }
          steps {
            sh '''
                export SVN_REVISION=`git log -1 | grep "\\S" | tail -n 1 | sed "s/.*@\\([0-9]*\\).*/\\1/"`
                ./gradlew clean publishPodspec -Prevision=$SVN_REVISION'''
          }
        }
      }
    }
  }
  post {
    always {
      cleanAndNotify()
    }
  }
}