Pod::Spec.new do |s|
  s.name = 'Giac'
  s.version = '${version}'
  s.summary = 'Giac library'
  s.platform = :ios, '11.0'
  s.source = { :http => 'https://dev.geogebra.org/maven2/fr/ujf-grenoble/javagiac/${version}/javagiac-${version}.zip'}
  s.requires_arc = false
  s.authors = 'Bernard Parisse', 'GeoGebra Team'
  s.homepage = 'http://dev.geogebra.org'
  s.license = 'Giac/LICENSE'
  s.preserve_paths = '*'
  s.source_files = 'Giac/headers/ContextBridge.hpp', 'Giac/headers/GenBridge.hpp'
  s.vendored_libraries = 'Giac/libs/libgiac.a', 'Giac/libs/libgmp.a', 'Giac/libs/libmpfr.a', 'Giac/libs/libsimpleInterface.a'
  
  s.pod_target_xcconfig = { 'EXCLUDED_ARCHS[sdk=iphonesimulator*]' => 'arm64' }
  s.user_target_xcconfig = { 'EXCLUDED_ARCHS[sdk=iphonesimulator*]' => 'arm64' }
end
