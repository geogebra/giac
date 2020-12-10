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
    EMSCRIPTEN_VER='201503'
    EMSCRIPTEN_VERSION='tag-1.34.1'
    ANDROID_SDK_ROOT='/var/lib/jenkins/.android-sdk'
  }
  stages {
    stage('Build') {
      steps {
        sh '''
          cp -r /data/backup-Giac/geogebra/giac/emsdk .
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
