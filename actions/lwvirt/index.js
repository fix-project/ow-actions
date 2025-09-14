"use strict"

const openwhisk = require('openwhisk');
const options = {
  api_key :
      '23bc46b1-71f6-4ed5-8c54-816aa4f8c502:123zO3xZCLrMN6v2BKK1dXYFpXlPkccOFqm12CdAsMgRU4VrNZ9lyGVCGuMDGIwP'
}

const ow = openwhisk(options)

const polling_delay = 0
const delay = async ms => new Promise(resolve => setTimeout(resolve, ms))

async function poll_act(activation) {
  let result = null
  try{
    do {
      try {
        result = await ow.activations.get({name : activation.activationId})
      } catch (err) {
      }

      if (!result) {
        await delay(polling_delay)
      }
    } while (!result);
  } catch (uncatchederr) {
    console.error( uncatchederr );
    throw uncatchederr
  }

  return result;
}

const invoke_delay = 0;
async function try_submit(activation) {
	let result = null
	try {
      	  do {
      	  	try { 
      	  		result = await ow.actions.invoke(activation);
      	  	} catch (err) {
      	  	}
      
      	  	if (!result) {
      	  		await delay(invoke_delay)
      	  	}
      	  } while (!result);
	} catch ( uncatchederr ) {
          console.error( uncatchederr );
          throw uncatchederr
	}

	return result;
}

var Minio = require('minio');

var minioClient = new Minio.Client({
	endPoint: '10.103.170.222',
	port: 80,
	useSSL: false,
	accessKey: 'minioadmin',
	secretKey: 'minioadmin',
});

const fs = require('fs');

async function create_add() {
	const name = 'add'

	try {
		const result = await ow.actions.get( name );
	} catch ( err ) {
		const get_add = await minioClient.fGetObject('actions', 'add.zip', 'add.zip')
		
		const add = fs.readFileSync('add.zip')

		try {
			const result = await ow.actions.create({
				"namespace": "guest",
				"name": name,
				"action": add,
				"exec": {
					"kind": "blackbox",
					"image": "openwhisk/dockerskeleton",
					"binary": true
				},
				"annotations": [
					{
						"key": "provide-api-key",
						"value": false
					},
					{
						"key": "exec",
						"value": "blackbox"
					}
				],
				"limits": {
					"timeout": 240000,
					"memory": 256,
					"logs": 10,
					"concurrency": 1
				}
			});
		} catch ( newerr ) {
			console.log( 'action concurrently created' );
		}
	}

	return name
}

async function lwvirt(params) {
  const add_created = await create_add();

  let activation = await try_submit({
	  name : add_created,
	  params : {
		  x : 0,
		  y : 0,
	  }
  });
  await poll_act( activation )

  const start = Date.now();
  for ( let i = 0; i < 4096; i++ ) {
	  let act = await try_submit({
		  name : add_created,
		  params: {
			  x : i / 254 + 1,
			  y : i % 254 + 1
		  }
	  });
	  await poll_act( act )
  }
  const end = Date.now();
  console.log(`Duration: `, end - start);
}

exports.main = lwvirt
