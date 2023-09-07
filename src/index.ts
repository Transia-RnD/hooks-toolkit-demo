import {
  Xrpld,
  createHookPayload,
  iHookParamEntry,
  iHookParamName,
  iHookParamValue,
  setHooksV3,
  SetHookParams,
  ExecutionUtility,
  clearAllHooksV3,
  floatToLEXfl,
} from '@transia/hooks-toolkit'
// NOT EXPORTED
import {
  XrplIntegrationTestContext,
  accountSet,
  close,
  serverUrl,
  setupClient,
  trust,
} from '@transia/hooks-toolkit/dist/npm/src/libs/xrpl-helpers'
import {
  Payment,
  TrustSet,
  Invoke,
  AccountSetAsfFlags,
  SetHookFlags,
  TransactionMetadata,
} from '@transia/xrpl'
import { IssuedCurrencyAmount } from '@transia/xrpl/dist/npm/models/common'
import { AccountID } from '@transia/ripple-binary-codec/dist/types'
import { sign } from '@transia/ripple-keypairs'

export async function setHooks(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hookWallet = testContext.alice
  const fmWallet = testContext.bob

  const txAmount1: IssuedCurrencyAmount = {
    value: '100000000',
    currency: 'USD',
    issuer: testContext.gw.classicAddress,
  }
  const builtTx1: TrustSet = {
    TransactionType: 'TrustSet',
    Account: hookWallet.classicAddress,
    LimitAmount: txAmount1,
  }
  await Xrpld.submit(testContext.client, {
    wallet: hookWallet,
    tx: builtTx1,
  })

  // SET HOOK IN - NAV
  const hook1Param1 = new iHookParamEntry(
    new iHookParamName('PK'),
    new iHookParamValue(fmWallet.publicKey, true)
  )
  const hook1 = createHookPayload(
    0,
    'nav',
    'nfo',
    SetHookFlags.hsfOverride,
    ['Invoke'],
    [hook1Param1.toXrpl()]
  )

  // SET HOOK IN - NFO
  const hook2 = createHookPayload(0, 'nfo', 'nfo', SetHookFlags.hsfOverride, [
    'Payment',
  ])

  // SET HOOK IN - 3MM
  const hookAcctHex = AccountID.from(hookWallet.classicAddress).toHex()
  const hook3Param1 = new iHookParamEntry(
    new iHookParamName('IMC'),
    new iHookParamValue('01', true)
  )
  const hook3Param2 = new iHookParamEntry(
    new iHookParamName('495300', true), // IS
    new iHookParamValue(hookAcctHex, true)
  )
  const hook3 = createHookPayload(
    0,
    '3mm',
    'nfo',
    SetHookFlags.hsfOverride,
    ['Invoke'],
    [hook3Param1.toXrpl(), hook3Param2.toXrpl()]
  )

  await setHooksV3({
    client: testContext.client,
    seed: hookWallet.seed,
    hooks: [{ Hook: hook1 }, { Hook: hook2 }, { Hook: hook3 }],
  } as SetHookParams)

  await testContext.client.disconnect()
}

export async function setNAV(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hookWallet = testContext.alice
  const fmWallet = testContext.bob

  // INVOKE IN
  const nav = String(100)
  const tx1Param1 = new iHookParamEntry(
    new iHookParamName('NAV'),
    new iHookParamValue(floatToLEXfl(nav), true)
  )
  const tx1Param2 = new iHookParamEntry(
    new iHookParamName('SIG'),
    new iHookParamValue(sign(floatToLEXfl(nav), fmWallet.privateKey), true)
  )

  const builtTx: Invoke = {
    TransactionType: 'Invoke',
    Account: fmWallet.classicAddress,
    Destination: hookWallet.classicAddress,
    HookParameters: [tx1Param1.toXrpl(), tx1Param2.toXrpl()],
  }
  await Xrpld.submit(testContext.client, {
    wallet: fmWallet,
    tx: builtTx,
  })
  await testContext.client.disconnect()
}

export async function buyNFO(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hookWallet = testContext.alice
  const client1Wallet = testContext.elsa

  // TRUST SET IN
  const txAmount1: IssuedCurrencyAmount = {
    value: '100000000',
    currency: 'NAV',
    issuer: hookWallet.classicAddress,
  }
  const builtTx1: TrustSet = {
    TransactionType: 'TrustSet',
    Account: client1Wallet.classicAddress,
    LimitAmount: txAmount1,
  }
  await Xrpld.submit(testContext.client, {
    wallet: client1Wallet,
    tx: builtTx1,
  })

  // PAYMENT IN
  const txAmount2: IssuedCurrencyAmount = {
    value: '100',
    currency: testContext.ic.currency as string,
    issuer: testContext.ic.issuer as string,
  }

  const builtTx2: Payment = {
    TransactionType: 'Payment',
    Account: client1Wallet.classicAddress,
    Destination: hookWallet.classicAddress,
    Amount: txAmount2,
  }
  const result2 = await Xrpld.submit(testContext.client, {
    wallet: client1Wallet,
    tx: builtTx2,
  })
  const result2Executions = await ExecutionUtility.getHookExecutionsFromMeta(
    testContext.client,
    result2.meta as TransactionMetadata
  )
  console.log(result2Executions.executions[0].HookReturnString)
  await close(testContext.client)
  await testContext.client.disconnect()
}

export async function main(): Promise<void> {
  await setHooks()
  await setNAV()
  await buyNFO()
}

main()
// .finally(async () => {
//   const testContext = (await setupClient(
//     serverUrl
//   )) as XrplIntegrationTestContext
//   await clearAllHooksV3({
//     client: testContext.client,
//     seed: testContext.alice.seed,
//   } as SetHookParams)
//   await testContext.client.disconnect()
// })
