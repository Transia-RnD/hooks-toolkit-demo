import {
  Xrpld,
  createHookPayload,
  setHooksV3,
  SetHookParams,
  ExecutionUtility,
  iHookParamEntry,
  iHookParamName,
  iHookParamValue,
  floatToLEXfl,
} from '@transia/hooks-toolkit'
// NOT EXPORTED
import {
  XrplIntegrationTestContext,
  close,
  serverUrl,
  setupClient,
} from '@transia/hooks-toolkit/dist/npm/src/libs/xrpl-helpers'
import { AccountID } from '@transia/ripple-binary-codec/dist/types'
import { IssuedCurrencyAmount } from '@transia/xrpl/dist/npm/models/common'
import { Payment, SetHookFlags, TransactionMetadata } from '@transia/xrpl'
import { SellerModel } from './models/SellerModel'
import { SellerArray } from './models/SellerArray'

// 1. Payment of 250
// 2 sellers
// Seller 1 gets 150
// Seller 2 gets 100

async function main(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hook = createHookPayload(0, 'base', 'base', SetHookFlags.hsfOverride, [
    'Payment',
  ])

  await setHooksV3({
    client: testContext.client,
    seed: testContext.alice.seed,
    hooks: [{ Hook: hook }],
  } as SetHookParams)

  // INVOKE IN
  const aliceWallet = testContext.alice
  const buyerWallet = testContext.bob
  const seller1Wallet = testContext.carol
  const seller2Wallet = testContext.dave

  const nestedModel = new SellerModel(10, 'rHb9CJAWyB4rj91VRWn96DkukG4bwdtyTh')

  const sample = new SellerArray([nestedModel])
  console.log(sample.encode())

  // const param1 = new iHookParamEntry(
  //   new iHookParamName('S1P'),
  //   new iHookParamValue(floatToLEXfl('150'), true)
  // )
  // const acctHex = AccountID.from(seller1Wallet.classicAddress)
  // const param2 = new iHookParamEntry(
  //   new iHookParamName('S1A'),
  //   new iHookParamValue(acctHex.toHex(), true)
  // )

  // // const param3 = new iHookParamEntry(
  // //   new iHookParamName('S2P'),
  // //   new iHookParamValue(floatToLEXfl('150'), true)
  // // )
  // // const param4 = new iHookParamEntry(
  // //   new iHookParamName('S2A'),
  // //   new iHookParamValue(floatToLEXfl('10'), true)
  // // )

  // const amount: IssuedCurrencyAmount = {
  //   issuer: testContext.ic.issuer as string,
  //   currency: testContext.ic.currency as string,
  //   value: '250',
  // }

  // const builtTx: Payment = {
  //   TransactionType: 'Payment',
  //   Account: buyerWallet.classicAddress,
  //   Destination: aliceWallet.classicAddress,
  //   Amount: amount,
  //   HookParameters: [param1.toXrpl(), param2.toXrpl()],
  // }
  // console.log(builtTx.HookParameters)

  // const result = await Xrpld.submit(testContext.client, {
  //   wallet: buyerWallet,
  //   tx: builtTx,
  // })
  // const hookExecutions = await ExecutionUtility.getHookExecutionsFromMeta(
  //   testContext.client,
  //   result.meta as TransactionMetadata
  // )
  // console.log(hookExecutions.executions[0].HookReturnString)

  // await close(testContext.client)
  await testContext.client.disconnect()
}

main()
