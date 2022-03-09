package fr.grenoble.ujf.giac;

import org.gwtproject.resources.client.ClientBundle;
import org.gwtproject.resources.client.Resource;
import org.gwtproject.resources.client.TextResource;


/**
 * CAS resource bundle
 */
@Resource
public interface CASResources extends ClientBundle {

	/** @return giac.wasm */
	@Source("fr/grenoble/ujf/giac/giac.wasm.js")
	TextResource giacWasm();

}
