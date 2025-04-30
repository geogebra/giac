pipeline {
  agent none
  options {
      buildDiscarder(logRotator(numToKeepStr: '30'))
  }

  stages {
    stage('Mac, Windows binaries') {
      parallel {
        stage('Win') {
          agent {label 'winbuild'}
          environment {
            MAVEN = credentials('maven-repo')
          }
          steps {
            bat "C:\\msys64\\usr\\bin\\env.exe MSYSTEM=CLANG64 C:\\msys64\\usr\\bin\\bash -l -c \"cd ${env.WORKSPACE.replace('\\','/').replace('C:','/c')}; bash ./recompile-msys.sh cbuild64\""
            stash name: "giac-clang", includes: "cbuild*/**"
          }
          post {
            always { deleteDir() }
          }
        }
        stage('Mac') {
          agent {label 'mac-mini'}
          environment {
            MAVEN = credentials('maven-repo')
          }
          stages {
            stage('Mac JNI') {
              steps {
                sh "rm src/giac/cpp/kdisplay.cc"
                sh "export ANDROID_SDK_ROOT=~/.android-sdk/; ./gradlew javagiacOsx_amd64SharedLibrary javagiacOsx_arm64SharedLibrary --info"
                stash name: 'giac-mac', includes: 'build/binaries/javagiacSharedLibrary/osx_x86-64/libjavagiac.jnilib'
                stash name: 'giac-mac-arm64', includes: 'build/binaries/javagiacSharedLibrary/osx_arm-v8/libjavagiac.jnilib'
              }
            }
            stage('Objective C') {
              steps {
                sh '''
                  export SVN_REVISION=`git log -1 | grep "\\S" | tail -n 1 | sed "s/.*@\\([0-9]*\\).*/\\1/"`
                  ./gradlew clean publishMavenZipPublicationToMavenRepository -Prevision=$SVN_REVISION'''
                }
            }
          }
          post {
            always { deleteDir() }
          }
        }
      }
    }
    stage('Build') {
      parallel {
        stage('Java and JS') {
          agent {label 'deploy2'}
          environment {
            MAVEN = credentials('maven-repo')
            ANDROID_SDK_ROOT='/var/lib/jenkins/.android-sdk'
            EM_BINARYEN_ROOT="${env.WORKSPACE}/emsdk/upstream"
            EMSDK_PYTHON='/usr/bin/python3.10'
            NDK="$ANDROID_SDK_ROOT/ndk/28.0.12916984"
            NDK_TOOLCHAIN="$NDK/toolchains/llvm/prebuilt/linux-x86_64"
            PATH="$NDK_TOOLCHAIN/bin:$NDK_TOOLCHAIN/sysroot/usr/lib/arm-linux-androideabi:/var/lib/jenkins/glibc/build/elf:$PATH"
          }
          steps {
            unstash name: 'giac-clang'
            unstash name: 'giac-mac'
            unstash name: 'giac-mac-arm64'
            sh "rm src/giac/cpp/kdisplay.cc"
            sh '''
               export SVN_REVISION=`git log -1 | grep "\\S" | tail -n 1 | sed "s/.*@\\([0-9]*\\).*/\\1/"`
              ./gradlew downloadEmsdk installEmsdk activateEmsdk
              ./gradlew :emccClean :giac-gwt:publish --no-daemon -Prevision=$SVN_REVISION --refresh-dependencies
              ./gradlew :updateGiac --no-daemon -Prevision=$SVN_REVISION --info'''
          }
          post {
            always { deleteDir() }
          }
        }
      }
    }
  }
  post {
    always {
      cleanAndNotify("#giac")
    }
  }
}