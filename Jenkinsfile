def crosscompilers = '/var/lib/jenkins/cross-compilers'

pipeline {
  agent none
  environment {
    MAVEN = credentials('maven')
    MAC = credentials('mac-giac')
    NPM = credentials('npm-registry')
    LINUX32_SSH=credentials('linux32-ssh-key')
    ANDROID_SDK_ROOT='/var/lib/jenkins/.android-sdk'
    BINARYEN="${env.WORKSPACE}/emsdk/upstream"
    EMSDK_PYTHON='/usr/bin/python3.8'
  }
  stages {
    stage('Build') {
      parallel {
        stage('Java and JS') {
          agent {label 'deploy2'}
          environment {
            PATH="$crosscompilers/x86/bin:$crosscompilers/x86_64/bin:$crosscompilers/arm/bin:$crosscompilers/arm64/bin:$PATH"
          }
          steps {
            sh '''
              ./gradlew downloadEmsdk installEmsdk activateEmsdk
              ./gradlew :emccClean :giac-gwt:publish --no-daemon -Prevision=$SVN_REVISION --info --refresh-dependencies
              ./gradlew :updateGiac --no-daemon -Prevision=$SVN_REVISION --info'''
          }
        }
        stage('Objective C') {
          agent {label 'mac'}
          steps {
            sh './gradlew clean publishPodspec -Prevision=$SVN_REVISION'
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
