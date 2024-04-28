"use strict"

const openwhisk = require('openwhisk');
const options = {
  api_key :
      '23bc46b1-71f6-4ed5-8c54-816aa4f8c502:123zO3xZCLrMN6v2BKK1dXYFpXlPkccOFqm12CdAsMgRU4VrNZ9lyGVCGuMDGIwP'
};
const ow = openwhisk(options);

const polling_delay = 500;
const delay = async ms => new Promise(resolve => setTimeout(resolve, ms));

async function poll_wasm2c(params) {
  const activation = await ow.actions.invoke({name : 'wasm2c', params : params});
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
  let activations = [] for (let i = 0; i < output_number; i++) {
    try {
      const act = await ow.actions.invoke({
        name : 'ctoelf',
        params : {bucket : params.output_bucket, index : i}
      });
      activations.push(act);
    } catch (err) {
      console.error(`Failed to invoke ctoelf`, err);
      return { msg: 'Failed to invoke ctoelf' }
    }
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
  console.log(params.output_bucket)
  console.log(output_number - 1)

  let result = null

  try {
    let activation = await ow.actions.invoke({
      name : 'linkelfs',
      params : {
        bucket : params.output_bucket,
        last_index : output_number - 1,
        output_name : 'out'
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

async function main(params) {
  const w2c_res = await poll_wasm2c(params);
  console.log(w2c_res.response.result.output_number)

  const ctoelf_res =
      await poll_ctoelf(params, w2c_res.response.result.output_number)
  const linkelfs_res =
      await poll_linkelfs(params, w2c_res.response.result.output_number)

  return linkelfs_res
}
