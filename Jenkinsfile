def crosscompilers = '/var/lib/jenkins/cross-compilers'

pipeline {
  agent {label 'deploy2'}
  environment {
    PATH="$crosscompilers/x86/bin:$crosscompilers/x86_64/bin:$crosscompilers/arm/bin:$crosscompilers/arm64/bin:$PATH"
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
      steps {
        sh '''
          ./gradlew downloadEmsdk installEmsdk activateEmsdk
          ./gradlew :emccClean :giac-gwt:publish --no-daemon -Prevision=$SVN_REVISION --info --refresh-dependencies
          ./gradlew :updateGiac --no-daemon -Prevision=$SVN_REVISION --info'''
        node('mac') {
          checkout([$class: 'SubversionSCM', 
            locations: [[cancelProcessOnExternalsFail: true, 
              credentialsId: 'svn', 
              depthOption: 'infinity', 
              ignoreExternalsOption: true, 
              local: '.', 
              remote: 'https://dev.geogebra.org/svn/trunk/geogebra/giac']], 
            quietOperation: true, 
            workspaceUpdater: [$class: 'UpdateUpdater']])
          sh './gradlew clean publishPodspec -Prevision=$SVN_REVISION'
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
