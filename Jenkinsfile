def crosscompilers = '/var/lib/jenkins/cross-compilers'

pipeline {
  agent {label 'deploy2'}
  environment {
    PATH="$crosscompilers/x86/bin:$crosscompilers/x86_64/bin:$crosscompilers/arm/bin:$crosscompilers/arm64/bin:$PATH"
    MAVEN = credentials('maven')
    MAC = credentials('mac-giac')
    NPM = credentials('npm-registry')
    WIN_BASH = credentials('win-bash')
    WIN_SSH=credentials('win-ssh')
    LINUX32_SSH=credentials('linux32-ssh-key')
    ANDROID_SDK_ROOT='/var/lib/jenkins/.android-sdk'
    BINARYEN="${env.WORKSPACE}/emsdk/upstream"
    EMSDK_PYTHON='/usr/bin/python3.7'
  }
  stages {
    stage('Build') {
      steps {
        sh '''
          test -x emsdk || git clone https://github.com/emscripten-core/emsdk.git
          ./gradlew installEmsdk
          ln -s emsdk emsdk/emsdk-portable
          mkdir emsdk/emscripten
          ln -sr emsdk/upstream/emscripten emsdk/emscripten/latest
          ./gradlew :emccClean :giac-gwt:publish --no-daemon -Prevision=$SVN_REVISION --info --refresh-dependencies
          ./gradlew :updateGiac :publishNodegiac --no-daemon -Prevision=$SVN_REVISION --info'''
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
          sh './gradlew publishPodspec -Prevision=$SVN_REVISION'
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
