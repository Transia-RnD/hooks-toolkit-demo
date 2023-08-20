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

  const hook = createHookPayload(0, '401k', '401k', SetHookFlags.hsfOverride, [
    'Payment',
  ])

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

// report()
setHookAccount().then(() => {
  // buyNAVAlice()
})
