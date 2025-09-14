"use strict"

const openwhisk = require('openwhisk');
const fs = require('fs');
const options = {
  api_key :
      '23bc46b1-71f6-4ed5-8c54-816aa4f8c502:123zO3xZCLrMN6v2BKK1dXYFpXlPkccOFqm12CdAsMgRU4VrNZ9lyGVCGuMDGIwP'
};
const ow = openwhisk(options);

const polling_delay = 500;
const delay = async ms => new Promise(resolve => setTimeout(resolve, ms));

async function poll_act(activation) {
  let result = null
  do {
    try {
      result = await ow.activations.get({name : activation.activationId})
    } catch (err) {
      if (err.statusCode !== 404) {
        throw err
      }
    }

    if (!result) {
      await delay(polling_delay)
    }
  } while (!result);

  return result;
}

const invoke_delay = 1000;
async function try_submit(activation) {
	let result = null
	do {
		try { 
			result = await ow.actions.invoke(activation);
		} catch (err) {
			if (err.statusCode !== 429) {
				throw err
			}
		}

		if (!result) {
			await delay(invoke_delay)
		}
	} while (!result);

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

async function create_wasm2c() {
	const name = 'wasm2c'

	try {
		const result = await ow.actions.get( name );
	} catch ( err ) {
		console.log('wasm2c does not exist')
		const getwasm2c = await minioClient.fGetObject('actions', 'wasm2c.zip', 'wasm2c.zip')
		
		const wasm2c = fs.readFileSync('wasm2c.zip')

		try {
			const result = await ow.actions.create({
				"namespace": "guest",
				"name": "wasm2c",
				"action": wasm2c,
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
					"memory": 8192,
					"logs": 10,
					"concurrency": 1
				}
			});
		} catch ( newerr ) {
			console.log( 'wasm2c concurrently created' );
		}
	}
}

async function create_linkelfs() {
	const name = 'linkelfs'
	try {
		const result = await ow.actions.get( name );
	} catch ( err ) {
		console.log('linkelfs does not exist')
		try {
			const result = await ow.actions.create({
				"namespace": "guest",
				"name": name,
				"action" : "",
				"exec": {
					"kind": "blackbox",
					"image": "yhdengh/fixpoint-benchmark:linkelfs",
					"binary": false
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
					"timeout": 120000,
					"memory": 8192,
					"logs": 10,
					"concurrency": 1
				},
			});
		} catch (newerr) {
			console.log( 'linkelfs concurrently created' );
		}
	}
}

async function create_ctoelf() {
	const name = 'newctoelf'
	try {
		const result = await ow.actions.get( name );
	} catch ( err ) {
		console.log('newctoelf does not exist')

		try {
			const result = await ow.actions.create({
				"namespace": "guest",
				"name": "newctoelf",
				"action" : "",
				"exec": {
					"kind": "blackbox",
					"image": "yhdengh/fixpoint-benchmark:ctoelf",
					"binary": false
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
					"memory": 1024,
					"logs": 10,
					"concurrency": 1
				},
			});
		} catch (newerr) {
			console.log( 'ctoelf concurrently created' );
		}
	}
}


async function poll_wasm2c(params) {
	const wasm2c_created = await create_wasm2c();
	const activation = await try_submit({name : 'wasm2c', params : params});
	let result = null
	do {
		try {
			result = await ow.activations.get({name : activation.activationId})
		} catch (err) {
			if (err.statusCode !== 404) {
				throw err
			}
		}

		await delay(polling_delay)
	} while (!result);

	return result;
}

async function poll_ctoelf(params, output_number) {
	const ctoelf_created = await create_ctoelf();
	let activations = [];
	for (let i = 0; i < output_number; i++) {
		const act = await try_submit({
			name : 'newctoelf',
			params : {bucket : params.output_bucket, 
				  index : i,
			          minio_url : params.minio_url}
		});
		activations.push(act);
	}

	for (const act of activations) {
		let result = null
		do {
			try {
				result = await ow.activations.get({name : act.activationId})
			} catch (err) {
				if (err.statusCode !== 404) {
					throw err
				}
			}

			if (!result) {
				await delay(polling_delay)
			}
		} while (!result);
	}

	return { msg: 'All ctoelf done' }
}

async function poll_linkelfs(params, output_number) {
	const linkelfs_created = await create_linkelfs();
	console.log(params.output_bucket)
	console.log(output_number - 1)

	let result = null

	try {
		let activation = await try_submit({
			name : 'linkelfs',
			params : {
				bucket : params.output_bucket,
				last_index : output_number - 1,
				output_name : 'out',
				minio_url : params.minio_url
			}
		});

		do {
			try {
				result = await ow.activations.get({name : activation.activationId})
			} catch (err) {
				if (err.statusCode !== 404) {
					throw err
				}
			}

			if (!result) {
				await delay(polling_delay)
			}
		} while (!result)

	} catch (err) {
		console.error(`Failed to invoke linkelfs`, err);
		return { msg: 'Failed to invoke linkelfs' }
	}

	return result
}

async function mycompilepoll(params) {
	const ctoelf_res =
		await poll_ctoelf(params, params.output_number)
	const linkelfs_res =
		await poll_linkelfs(params, params.output_number)

	return linkelfs_res
}

exports.main = mycompilepoll;
