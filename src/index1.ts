import {
  Xrpld,
  createHookPayload,
  setHooksV3,
  SetHookParams,
  clearAllHooksV3,
} from '@transia/hooks-toolkit'
// NOT EXPORTED
import {
  XrplIntegrationTestContext,
  accountSet,
  serverUrl,
  setupClient,
} from '@transia/hooks-toolkit/dist/npm/src/libs/xrpl-helpers'
import { Payment, SetHookFlags } from '@transia/xrpl'
import { IssuedCurrencyAmount } from '@transia/xrpl/dist/npm/models/common'

export async function test(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hookWallet = testContext.alice
  const client1Wallet = testContext.elsa

  const hook1 = createHookPayload(0, 'base', 'base', SetHookFlags.hsfOverride, [
    'Payment',
  ])
  await setHooksV3({
    client: testContext.client,
    seed: hookWallet.seed,
    hooks: [{ Hook: hook1 }],
  } as SetHookParams)

  // PAYMENT IN
  const txAmount2: IssuedCurrencyAmount = {
    value: '100',
    currency: testContext.ic.currency as string,
    issuer: testContext.ic.issuer as string,
  }
  const tx: Payment = {
    TransactionType: 'Payment',
    Account: client1Wallet.classicAddress,
    Destination: hookWallet.classicAddress,
    Amount: txAmount2,
  }
  await Xrpld.submit(testContext.client, {
    wallet: client1Wallet,
    tx: tx,
  })
  await testContext.client.disconnect()
}

export async function main(): Promise<void> {
  await test()
}

main()

export async function clear(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext
  await clearAllHooksV3({
    client: testContext.client,
    seed: testContext.alice.seed,
  } as SetHookParams)
  await testContext.client.disconnect()
}

// clear()
