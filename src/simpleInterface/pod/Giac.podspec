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
  s.source_files = 'Giac/headers/ContextBridge.hpp', 'Giac/headers/GenBridge.hpp'
  s.vendored_frameworks = "Giac.xcframework"
end
