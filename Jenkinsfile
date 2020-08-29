def crosscompilers = '/var/lib/jenkins/cross-compilers'

pipeline {
  agent {label 'deploy2'}
  environment {
    PATH="$crosscompilers/x86/bin:$crosscompilers/x86_64/bin:$crosscompilers/arm/bin:$cross-compilers/arm64/bin:$PATH"
    MAVEN = credentials('maven')
    MAC = credentials('mac-giac')
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
        sh """cd /var/lib/jenkins/workspace/Giac/
          svn checkout https://dev.geogebra.org/svn/trunk/geogebra/giac
          cd giac
          mkdir -p keys
          cp '$LINUX32_SSH' keys/compileLinux32JavagiacKey"""

        sh '''cd /var/lib/jenkins/workspace/Giac/giac
          REVISION=`svn info . | grep Revision | sed \'s/Revision: //\' | tr \'\\n\' \'\\0\'`
          cp -r ~/workspace/backup-Giac/geogebra/giac/emsdk .
          ./gradlew :clean :giac-android:clean :giac-gwt:clean --refresh-dependencies --no-daemon -Prevision=$REVISION --info
          ./gradlew :emccClean :giac-gwt:publish --no-daemon -Prevision=$REVISION --info
          ./gradlew :updateGiac :publishNodegiac --no-daemon -Prevision=$REVISION --info'''
      }
    }
  }
  post {
    always {
      echo 'cleanAndNotify()'
    }
  }
}
