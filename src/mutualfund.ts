import {
  Wallet,
  Payment,
  TrustSet,
  SetHookFlags,
  TransactionMetadata,
  xrpToDrops,
} from '@transia/xrpl'
import { IssuedCurrencyAmount } from '@transia/xrpl/dist/npm/models/common'
import { AccountID, Currency } from '@transia/ripple-binary-codec/dist/types'

import {
  createHookPayload,
  setHooksV3,
  SetHookParams,
  Xrpld,
  ExecutionUtility,
} from '@transia/hooks-toolkit'

// NOT EXPORTED
import {
  IC,
  XrplIntegrationTestContext,
  balance,
  close,
  serverUrl,
  setupClient,
  trust,
} from '@transia/hooks-toolkit/dist/npm/src/libs/xrpl-helpers'

export async function setHookAccount(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hookWallet = Wallet.fromSeed('sEd7QevWKtfC4nk2C6NxqyUU2pTViSc')
  if ((await balance(testContext.client, hookWallet.classicAddress)) < 2000) {
    const payTx: Payment = {
      TransactionType: 'Payment',
      Account: testContext.master.classicAddress,
      Destination: hookWallet.classicAddress,
      Amount: xrpToDrops(2000),
    }
    await Xrpld.submit(testContext.client, {
      wallet: testContext.master,
      tx: payTx,
    })

    const limitAmount: IssuedCurrencyAmount = {
      value: '10000',
      currency: testContext.ic.currency as string,
      issuer: testContext.ic.issuer as string,
    }
    const trustTx: TrustSet = {
      TransactionType: 'TrustSet',
      Account: hookWallet.classicAddress,
      LimitAmount: limitAmount,
    }
    await Xrpld.submit(testContext.client, {
      wallet: hookWallet,
      tx: trustTx,
    })
  }

  const hook = createHookPayload(
    0,
    'mutualfund_io',
    'mutualfund_io',
    SetHookFlags.hsfOverride,
    ['Payment']
  )

  await setHooksV3({
    client: testContext.client,
    seed: hookWallet.seed,
    hooks: [{ Hook: hook }],
  } as SetHookParams)

  await testContext.client.disconnect()
}

export async function buyNAVAlice(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hookWallet = Wallet.fromSeed('sEd7QevWKtfC4nk2C6NxqyUU2pTViSc')

  const aliceWallet = testContext.alice

  const nav = IC.gw('NAV', hookWallet.classicAddress)
  if (
    (await balance(testContext.client, aliceWallet.classicAddress, nav)) < 10000
  ) {
    console.log('ADD NAV TRUSTLINE')
    const limitAmount1: IssuedCurrencyAmount = {
      value: '10000',
      currency: nav.currency as string,
      issuer: nav.issuer as string,
    }
    const trustTx2: TrustSet = {
      TransactionType: 'TrustSet',
      Account: aliceWallet.classicAddress,
      LimitAmount: limitAmount1,
    }
    await Xrpld.submit(testContext.client, {
      wallet: aliceWallet,
      tx: trustTx2,
    })
  }

  // PAYMENT IN

  const amount: IssuedCurrencyAmount = {
    value: '100',
    currency: testContext.ic.currency as string,
    issuer: testContext.ic.issuer as string,
  }

  const builtTx: Payment = {
    TransactionType: 'Payment',
    Account: aliceWallet.classicAddress,
    Destination: hookWallet.classicAddress,
    Amount: amount,
  }

  const result = await Xrpld.submit(testContext.client, {
    wallet: aliceWallet,
    tx: builtTx,
  })

  await close(testContext.client)

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
  const hookWallet = Wallet.fromSeed('sEd7QevWKtfC4nk2C6NxqyUU2pTViSc')
  const nav = IC.gw('NAV', hookWallet.classicAddress)
  const hookXRPBal = await balance(
    testContext.client,
    hookWallet.classicAddress
  )
  const hookUSDBal = await balance(
    testContext.client,
    hookWallet.classicAddress,
    testContext.ic
  )
  const aliceXRPBal = await balance(
    testContext.client,
    testContext.alice.classicAddress
  )
  const aliceUSDBal = await balance(
    testContext.client,
    testContext.alice.classicAddress,
    testContext.ic
  )
  const aliceNAVBal = await balance(
    testContext.client,
    testContext.alice.classicAddress,
    nav
  )
  console.log(`HOOK XRP: ${hookXRPBal}`)
  console.log(`HOOK USD: ${hookUSDBal}`)
  console.log(`ALICE XRP: ${aliceXRPBal}`)
  console.log(`ALICE USD: ${aliceUSDBal}`)
  console.log(`ALICE NAV: ${aliceNAVBal}`)
  await testContext.client.disconnect()
}

// report()
setHookAccount().then(() => {
  // buyNAVAlice()
})
