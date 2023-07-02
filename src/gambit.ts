import {
  Client,
  Invoke,
  Payment,
  SetHookFlags,
  TransactionMetadata,
  xrpToDrops,
} from '@transia/xrpl'
import { AccountID } from '@transia/ripple-binary-codec/dist/types'
import {
  createHookPayload,
  setHooksV3,
  SetHookParams,
  iHookParamEntry,
  iHookParamName,
  iHookParamValue,
  Xrpld,
  ExecutionUtility,
  StateUtility,
  floatToLEXfl,
  iHookParamEntries,
} from '@transia/hooks-toolkit'

import {
  XrplIntegrationTestContext,
  balance,
  close,
  serverUrl,
  setupClient,
} from '@transia/hooks-toolkit/dist/npm/src/libs/xrpl-helpers'

import { decodeModel } from '@transia/hooks-toolkit/dist/npm/src/libs/binary-models'

import { GambitSlip } from './GambitSlip'
import { GambitBet } from './GambitBet'

export async function createBetAlice(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const testBet = new GambitBet(
    0, // 0 (Deployed), 1 (Ended) or 2 (Settled)
    floatToLEXfl('2') as unknown as bigint, // Odds Ex: 2:1
    'Vet adds a Hook to his twitter handle', // 150 chars
    0 // 0 (none), 1 (offer) or 2 (counter-offer)
  )
  console.log(testBet.encode())

  const param1 = new iHookParamEntry(
    new iHookParamName('GBC'),
    new iHookParamValue(testBet.encode(), true)
  )

  const hook = createHookPayload(
    0,
    'gambit',
    'gambit',
    SetHookFlags.hsfOverride,
    ['Payment', 'Invoke'],
    [param1.toXrpl()]
  )

  await setHooksV3({
    client: testContext.client,
    seed: testContext.alice.seed,
    hooks: [{ Hook: hook }],
  } as SetHookParams)

  await close(testContext.client)

  await testContext.client.disconnect()
}

export async function submitSlipBob(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  // const hook = createHookPayload(
  //   0,
  //   'gambit',
  //   'gambit',
  //   SetHookFlags.hsfOverride,
  //   ['Payment']
  // )

  // await setHooksV3({
  //   client: testContext.client,
  //   seed: testContext.alice.seed,
  //   hooks: [{ Hook: hook }],
  // } as SetHookParams)

  // INVOKE IN
  const aliceWallet = testContext.alice
  const bobWallet = testContext.bob

  const testModel = new GambitSlip(
    1, // 0 (none), 1 (offer) or 2 (counter-offer)
    BigInt(0),
    BigInt(0),
    BigInt(0),
    0,
    0
  )

  const param1 = new iHookParamEntry(
    new iHookParamName('GS'),
    new iHookParamValue(testModel.encode(), true)
  )

  const builtTx: Payment = {
    TransactionType: 'Payment',
    Account: bobWallet.classicAddress,
    Destination: aliceWallet.classicAddress,
    Amount: xrpToDrops(10),
    HookParameters: [param1.toXrpl()],
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
  const hookStateDir = await StateUtility.getHookStateDir(
    testContext.client,
    testContext.alice.classicAddress,
    'gambit'
  )
  console.log(hookStateDir)
  await testContext.client.disconnect()
}

export async function submitSlipCarol(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hook = createHookPayload(
    0,
    'gambit',
    'gambit',
    SetHookFlags.hsfOverride,
    ['Payment']
  )

  await setHooksV3({
    client: testContext.client,
    seed: testContext.alice.seed,
    hooks: [{ Hook: hook }],
  } as SetHookParams)

  // INVOKE IN
  const aliceWallet = testContext.alice
  const carolWallet = testContext.carol

  const testModel = new GambitSlip(
    2, // 0 (none), 1 (offer) or 2 (counter-offer)
    BigInt(0),
    BigInt(0),
    BigInt(0),
    0,
    0
  )

  const param1 = new iHookParamEntry(
    new iHookParamName('GS'),
    new iHookParamValue(testModel.encode(), true)
  )

  const builtTx: Payment = {
    TransactionType: 'Payment',
    Account: carolWallet.classicAddress,
    Destination: aliceWallet.classicAddress,
    Amount: xrpToDrops(20),
    HookParameters: [param1.toXrpl()],
  }
  const result = await Xrpld.submit(testContext.client, {
    wallet: carolWallet,
    tx: builtTx,
  })

  const hookExecutions = await ExecutionUtility.getHookExecutionsFromMeta(
    testContext.client,
    result.meta as TransactionMetadata
  )
  console.log(hookExecutions.executions[0].HookReturnString)
  const hookStateDir = await StateUtility.getHookStateDir(
    testContext.client,
    testContext.alice.classicAddress,
    'gambit'
  )
  console.log(hookStateDir)
  await testContext.client.disconnect()
}

export async function settle(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hook = createHookPayload(
    0,
    'gambit',
    'gambit',
    SetHookFlags.hsfOverride,
    ['Payment', 'Invoke']
  )

  await setHooksV3({
    client: testContext.client,
    seed: testContext.alice.seed,
    hooks: [{ Hook: hook }],
  } as SetHookParams)

  // INVOKE IN
  const aliceWallet = testContext.alice
  const carolWallet = testContext.carol

  const param1 = new iHookParamEntry(
    new iHookParamName('GZ'),
    new iHookParamValue(floatToLEXfl('2'), true)
  )

  const builtTx: Invoke = {
    TransactionType: 'Invoke',
    Account: carolWallet.classicAddress,
    Destination: aliceWallet.classicAddress,
    HookParameters: [param1.toXrpl()],
  }
  const result = await Xrpld.submit(testContext.client, {
    wallet: carolWallet,
    tx: builtTx,
  })

  const hookExecutions = await ExecutionUtility.getHookExecutionsFromMeta(
    testContext.client,
    result.meta as TransactionMetadata
  )
  console.log(hookExecutions.executions[0].HookReturnString)
  await testContext.client.disconnect()
}

export async function report(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  console.log(
    `BET BALANCE: ${
      (await balance(testContext.client, testContext.alice.classicAddress)) /
      1000000
    }`
  )

  const hook = await StateUtility.getHook(
    testContext.client,
    testContext.alice.classicAddress
  )
  const hookDefinition = await StateUtility.getHookDefinition(
    testContext.client,
    hook.Hooks[0].Hook.HookHash as string
  )
  // const ihookParamEntries = new iHookParamEntries(hookDefinition.HookParameters)
  // @ts-expect-error -- Accessing private property for testing
  console.log(hookDefinition.HookParameters[0].HookParameter.HookParameterName)
  console.log(
    decodeModel(
      // @ts-expect-error -- Accessing private property for testing
      hookDefinition.HookParameters[0].HookParameter
        .HookParameterValue as string,
      GambitBet
    )
  )

  const hookStateDir = await StateUtility.getHookStateDir(
    testContext.client,
    testContext.alice.classicAddress,
    'gambit'
  )
  // console.log(hookStateDir)

  // @ts-expect-error -- Accessing private property for testing
  for (let index = 0; index < hookStateDir.length; index++) {
    // @ts-expect-error -- Accessing private property for testing
    const element = hookStateDir[index]
    const key = element.HookStateKey as string
    const data = element.HookStateData as string

    const account = AccountID.from(
      key.split('000000000000000000000000').pop() as string
    )
    console.log(account.toJSON())
    console.log(data)
    console.log(decodeModel(data, GambitSlip))
  }
  await testContext.client.disconnect()
}

report()
// createBetAlice().then(() => {
//   submitSlipBob()
//   submitSlipCarol()
// })
// settle()

// Bet

// const bet = new GambitBet(
//   0, // 0 (Deployed), 1 (Ended) or 2 (Settled)
//   floatToLEXfl('2') as unknown as bigint, // Odds Ex: 2:1
//   'Vet adds a Hook to his twitter handle', // 150 chars
//   0 // 0 (none), 1 (offer) or 2 (counter-offer)
// )
// console.log(bet)
// console.log(bet.encode())

// console.log(
//   decodeModel(
//     '0000008D49FD1A8754255665742061646473206120486F6F6B20746F2068697320747769747465722068616E646C65000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000',
//     GambitSlip
//   )
// )

// // Slip

// const slip = new GambitSlip(
//   2,
//   6108881094714392576n,
//   6107881094714392576n,
//   BigInt(0),
//   1,
//   0
// )
// console.log(slip)
// console.log(slip.encode())

// console.log(
//   decodeModel(
//     '0254C71AFD498D000054C38D7EA4C680000000000000000000010000000000000000',
//     GambitSlip
//   )
// )
