Pod::Spec.new do |s|
  s.name = 'Giac'
  s.version = '${version}'
  s.summary = 'Giac library'
  s.platform = :ios, '11.0'
  s.source = { :http => 'https://dev.geogebra.org/maven2/fr/ujf-grenoble/igiac/${version}/igiac-${version}.zip'}
  s.requires_arc = false
  s.authors = 'Bernard Parisse', 'GeoGebra Team'
  s.homepage = 'http://dev.geogebra.org'
  s.license = 'Giac/LICENSE'
  s.vendored_frameworks = "Frameworks/Giac.xcframework"
end
