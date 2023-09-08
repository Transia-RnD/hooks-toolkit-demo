import {
  Xrpld,
  createHookPayload,
  setHooksV3,
  SetHookParams,
  ExecutionUtility,
} from '@transia/hooks-toolkit'
// NOT EXPORTED
import {
  XrplIntegrationTestContext,
  serverUrl,
  setupClient,
} from '@transia/hooks-toolkit/dist/npm/src/libs/xrpl-helpers'
import { Invoke, SetHookFlags, TransactionMetadata } from '@transia/xrpl'

async function main(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext
  const hook = createHookPayload(0, 'base', 'base', SetHookFlags.hsfOverride, [
    'Invoke',
  ])
  await setHooksV3({
    client: testContext.client,
    seed: testContext.alice.seed,
    hooks: [{ Hook: hook }],
  } as SetHookParams)

  // INVOKE IN
  const aliceWallet = testContext.alice
  const bobWallet = testContext.bob
  const builtTx: Invoke = {
    TransactionType: 'Invoke',
    Account: bobWallet.classicAddress,
    Destination: aliceWallet.classicAddress,
  }
  const result = await Xrpld.submit(testContext.client, {
    wallet: bobWallet,
    tx: builtTx,
  })
  const hookExecutions = await ExecutionUtility.getHookExecutionsFromMeta(
    testContext.client,
    result.meta as TransactionMetadata
  )
  console.log(hookExecutions.executions[0].HookReturnString)
  await testContext.client.disconnect()
}

main()
